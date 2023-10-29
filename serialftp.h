/* SPDX-License-Identifier: MIT */

/*
 * TODO:
 * - allow changing start/end words (default XFS/XSE)
 * - use checksum also for file upload
 */

#pragma once

#define SERIALFTP_MAX_PATH_LEN 256
#define SERIALFTP_MAX_FILE_SIZE 4096

struct SerialFTP
{
	enum Mode {
		MODE_FS,
		MODE_SERIAL
	};

	SerialFTP(void);
	void begin(void);
	void ls(void);
	void help(void);
	void put(char *buf, size_t len);
	void rm(char *buf, size_t len);
	void get(char *buf, size_t len);
	void loop(char *buf, size_t len);
	void run(void);

	protected:
	void handle_line(char *buf, size_t len);
	void write_size(size_t size);
	void write_checksum(char *buf, size_t size);
	void write_file(char *buf, size_t size);
	void set_cwd(const char *buf, size_t len);
	bool mkpath(char *buf, size_t len, char *path, size_t pathlen);

	public:
	char cwd[SERIALFTP_MAX_PATH_LEN];
	size_t cwd_len;
	enum Mode mode;
	char file_buffer[SERIALFTP_MAX_FILE_SIZE];
};
