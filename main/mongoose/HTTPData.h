/*
 * HTTPData.h
 *
 *  Created on: 22 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_MONGOOSE_HTTPDATA_H_
#define MAIN_MONGOOSE_HTTPDATA_H_

#include "mongoose.h"

class HTTPData {
public:
	HTTPData(struct http_message *_hm, struct mg_connection *_nc);
	void basicResponse(int code, const char *msg);

	void getBody(char *b, int maxsize=32000);
	int getHeader(const char *header, char *value, int maxsize=32000);
	int methodIs(const char *m, int maxsize=32000);
	void getUrl(char *url, int maxsize=32000);
	void getSourceIp(char *ip, int maxsize=32000);
	void getSourcePort(char *port, int maxsize=32000);
	void getSourceIpPort(char *ip, int maxsize=32000);
	void setResponseCode(int code);
	int getGET(const char *name, char *value, int maxsize=32000);
	int getPOST(const char *name, char *value, int maxsize=32000);

	static const char* code2msg(int code);
	struct CodeList {
		int code; char msg[32];
	};
	static const CodeList codelist[];

	static const char* ext2contentType(const char *name);
	struct ExtList {
		char ext[5]; char type[30];
	};
	static const ExtList extlist[];

	int printf(const char *fmt, ...);
	void send(char *ptr, int size);
private:
	struct http_message *hm;
	struct mg_connection *nc;
};



#endif /* MAIN_MONGOOSE_HTTPDATA_H_ */
