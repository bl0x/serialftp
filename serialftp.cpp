/* SPDX-License-Identifier: MIT */

#include <Arduino.h>
#include <serialftp.h>
#include <LittleFS.h>

SerialFTP::SerialFTP()
	: cwd_len(0)
	, mode(SerialFTP::MODE_SERIAL)
{}

void
SerialFTP::set_cwd(const char *buf, size_t len)
{
	size_t final_len;
	if (len < SERIALFTP_MAX_PATH_LEN) {
		final_len = snprintf(cwd, SERIALFTP_MAX_PATH_LEN, "/");
		if (final_len >= 0) {
			cwd_len = final_len;
		} else {
			cwd_len = 0;
		}
	}
}

bool
valid_filename(const char *buf, size_t len)
{
	for (int i = 0; i < len; ++i) {
		if (!(isalnum(buf[i]) || buf[i] == '/'
		    || buf[i] == '.' || buf[i] == '_' || buf[i] == '-')) {
			Serial.print("File '");
			Serial.print(buf);
			Serial.println("' is an invalid file name.");
			return false;
		}
	}
	return true;
}

void
init_littlefs()
{
	bool rc;
	rc = LittleFS.begin();
	if (rc == false) {
		Serial.println("FAIL: Could not mount LittleFS.\n");
	} else {
		Serial.println("OK  : LittleFS.\n");
	}
}

void
SerialFTP::begin()
{
	init_littlefs();
	set_cwd("/", 1);
}

void
SerialFTP::ls()
{
	Dir d;
	d = LittleFS.openDir("/");
	Serial.println("Listing '/':\n");
	while (d.next()) {
		//Serial.print("name: ");
		auto name = d.fileName();
		/*
		for (int i = 0; i < name.length(); ++i) {
			Serial.print(name[i], HEX);
			Serial.print(" ");
		}
		Serial.print("= \n\n");
		*/
		if (!valid_filename(name.c_str(), name.length())) {
			//Serial.println("Not a valid file name. Removing!");
			LittleFS.remove(d.fileName());
		}

		Serial.print(d.fileName());
		if (d.fileSize()) {
			File f = d.openFile("r");
			Serial.print(" \t");
			Serial.println(f.size());
		} else {
			if (d.isDirectory()) {
				Serial.println("/");
			} else {
				Serial.println(" \t--");
			}
		}
	}
}

bool
SerialFTP::mkpath(char *buf, size_t len, char *path, size_t pathlen)
{
	/* strip trailing whitespace */
	for (int i = len - 1; i >=0; ++i) {
		if (isspace(buf[i])) {
			len--;
		} else {
			break;
		}
	}
	buf[len] = '\0';

	if (!valid_filename(buf, len)) {
		return false;
	}

	if (cwd_len + 1 + len >= SERIALFTP_MAX_PATH_LEN) {
		Serial.println("Path too long.");
		return false;
	}
	if (len > 1 && buf[0] == '/') {
		snprintf(path, SERIALFTP_MAX_PATH_LEN, "%s", buf);
	} else if (cwd_len >= 1 && cwd[cwd_len - 1] == '/') {
		snprintf(path, SERIALFTP_MAX_PATH_LEN, "%s%s", cwd, buf);
	} else {
		snprintf(path, SERIALFTP_MAX_PATH_LEN, "%s/%s", cwd, buf);
	}

	return true;
}

void
SerialFTP::put(char *buf, size_t len)
{
	File f;
	char path[SERIALFTP_MAX_PATH_LEN];
	bool ok;

	ok = mkpath(buf, len, path, SERIALFTP_MAX_PATH_LEN);
	if (!ok) {
		return;
	}

	/*
	Serial.print("path = ");
	Serial.println(path);
	*/

	f = LittleFS.open(path, "w");
	if (f) {
		bool seen_end = false;
		int wr = 0;
		if (f.isDirectory()) {
			Serial.print("File '");
			Serial.print(path);
			Serial.println("' is a directory.");
			return;
		}
		Serial.println("OKGO");
		while (!seen_end) {
			int byte = Serial.read();
			if (byte == -1) {
				continue;
			}
			if (byte == 4 /* EOT */) {
				seen_end = true;
				break;
			}
			if (byte >= 0 && byte < 256) {
				file_buffer[wr] = (char)byte;
				wr++;
			}
		}
		f.write(file_buffer, wr);

		/*
		wr += f.write("This is a new file\r\n");
		wr += f.write("It has no reasonable content\r\n");
		wr += f.write("But it has UNIX line endings\r\n");
		*/

		f.close();
		Serial.print("Wrote ");
		Serial.print(wr);
		Serial.print(" bytes to file '");
		Serial.print(path);
		Serial.println("'");
	} else {
		Serial.print("Could not open file '");
		Serial.print(path);
		Serial.println("' for writing.");
	}
}

void
SerialFTP::write_size(size_t size)
{
	int i;
	uint8_t *p = (uint8_t *)&size;
	for (i = 0; i < sizeof(size); ++i) {
		Serial.write(*p++);
	}
}

void
SerialFTP::write_checksum(char *buf, size_t size)
{
	int i;
	uint8_t sum = 0;
	for (i = 0; i < size; ++i) {
		sum ^= (uint8_t)buf[i];
	}
	Serial.print("XOR");
	Serial.write(sum);
}

void
SerialFTP::write_file(char *buf, size_t size)
{
	int i;
	Serial.print("BEGIN");
	write_size(size);
	for (i = 0; i < size; ++i) {
		Serial.write(buf[i]);
	}
	write_checksum(file_buffer, size);
	Serial.print("EOF");
}

void
SerialFTP::rm(char *buf, size_t len)
{
	File f;
	char path[SERIALFTP_MAX_PATH_LEN];
	bool ok;

	ok = mkpath(buf, len, path, SERIALFTP_MAX_PATH_LEN);
	if (!ok) {
		return;
	}

	if (!LittleFS.exists(path)) {
		Serial.print("File '");
		Serial.print(path);
		Serial.println("' does not exist.");
		return;
	}

	LittleFS.remove(path);
	Serial.println("OK");
}

void
SerialFTP::get(char *buf, size_t len)
{
	File f;
	char path[SERIALFTP_MAX_PATH_LEN];
	bool ok;

	ok = mkpath(buf, len, path, SERIALFTP_MAX_PATH_LEN);
	if (!ok) {
		return;
	}

	if (!LittleFS.exists(path)) {
		Serial.print("File '");
		Serial.print(path);
		Serial.println("' does not exist.");
		return;
	}
	f = LittleFS.open(path, "r");
	if (f) {
		size_t size;
		size = f.size();
		if (size >= SERIALFTP_MAX_FILE_SIZE) {
			/* TODO: Handle large files. */
			Serial.print("File '");
			Serial.print(path);
			Serial.println("' is too large.");
			return;
		}
		if (f.isDirectory()) {
			Serial.print("File '");
			Serial.print(path);
			Serial.println("' is a directory.");
			return;
		}

		f.readBytes(file_buffer, size);
		write_file(file_buffer, size);
	} else {
		Serial.print("Could not open file '");
		Serial.print(path);
		Serial.println("' for reading.");
	}
}

void
SerialFTP::help()
{
	Serial.println("serial file system commands:");
	Serial.println("---------------------------------------------------");
	Serial.println("");
	Serial.println("help        - print this help");
	Serial.println("ls          - list contents of current directory");
	Serial.println("cd  <dir>   - change current directory to <dir>");
	Serial.println("get <file>  - get file named <file>");
	Serial.println("put <file>  - put local file into current directory");
	Serial.println("rm  <file>  - remove file");
	Serial.println("XSE         - leave file system mode");
	Serial.println("XFS         - enter file system mode");
	Serial.println("");
}

void
SerialFTP::handle_line(char *buf, size_t len)
{
	if (strncmp(buf, "help", 4) == 0) {
		help();
	} else if (strncmp(buf, "ls", 2) == 0) {
		ls();
	} else if (strncmp(buf, "get", 3) == 0) {
		size_t min_len = 4;
		char *arg_buf = &buf[min_len];
		if (len > min_len) {
			size_t arg_len = strlen(arg_buf);
			get(arg_buf, arg_len);
		} else {
			Serial.println("get requires an argument.");
		}
	} else if (strncmp(buf, "rm", 2) == 0) {
		size_t min_len = 3;
		char *arg_buf = &buf[min_len];
		if (len > min_len) {
			size_t arg_len = strlen(arg_buf);
			rm(arg_buf, arg_len);
		} else {
			Serial.println("rm requires an argument.");
		}
	} else if (strncmp(buf, "put", 3) == 0) {
		size_t min_len = 4;
		char *arg_buf = &buf[min_len];
		if (len > min_len) {
			size_t arg_len = strlen(arg_buf);
			put(arg_buf, arg_len);
		} else {
			Serial.println("put requires an argument.");
		}
	} else {
		Serial.print("Unknown command '");
		Serial.print(buf);
		Serial.println("'");
	}
}

void
SerialFTP::loop(char *buf, size_t len)
{
	if (strncmp(buf, "XFS", 3) == 0) {
		mode = MODE_FS;
		Serial.println("Switch to FS mode.");
		return;
	} else if (strncmp(buf, "XSE", 3) == 0) {
		mode = MODE_SERIAL;
		Serial.println("Switch to serial mode.");
		return;
	}

	if (mode == MODE_FS) {
		handle_line(buf, len);
	}
}

void
SerialFTP::run(void)
{
	char buf[200];
	char *cmd = NULL;
	int len = 0;
	bool cmd_complete = false;

	if (mode == MODE_SERIAL) {
		/* TODO: Run a callback */
	}

	cmd = buf;
	len = 0;
	cmd_complete = false;
	if (Serial.available()) {
read_more:
		while (Serial.available()) {
			char byte = Serial.read();
			if (byte == '\n') {
				cmd_complete = true;
				break;
			} else {
				cmd[len] = byte;
				++len;
			}
		}
		cmd[len] = '\0';

		if (!cmd_complete) {
			goto read_more;
		}

		loop(buf, len);
		/* TODO: Run a callback, if command not handled */
	}
}
