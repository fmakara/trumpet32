/*
 * HTTPData.cpp
 *
 *  Created on: 22 de mai de 2018
 *      Author: makara
 */

#include "HTTPData.h"
#include <stdio.h>

HTTPData::HTTPData(struct http_message *_hm, struct mg_connection *_nc):
		hm(_hm),nc(_nc) {
}

void HTTPData::getBody(char *b, int maxsize){
	if(hm->body.len<maxsize)maxsize=hm->body.len;
	memcpy(b,hm->body.p,maxsize);
	b[maxsize]=0;
}

int HTTPData::getHeader(const char *header, char *value, int maxsize){
	int i, j, index=-1;
	for(i=0;i<40 && hm->header_names[i].len>0;i++){
		for(j=0;j<hm->header_names[i].len;j++){
			if(hm->header_names[i].p[j]!=header[j]){
				j = 1000;
				break;
			}
		}
		if(j<1000){
			index = i;
			break;
		}
	}
	if(index<0)return 0;
	if(hm->header_names[index].len<maxsize)maxsize=hm->header_names[index].len;
	memcpy(value,hm->header_names[index].p,maxsize);
	value[maxsize]=0;
	return index+1;
}

int HTTPData::methodIs(const char *m, int maxsize){
	int j;
	for(j=0;j<hm->method.len;j++){
		if(hm->method.p[j]!=m[j])return 0;
	}
	return m[hm->method.len]==0;
}

void HTTPData::getUrl(char *url, int maxsize){
	if(hm->uri.len<maxsize)maxsize=hm->uri.len;
	memcpy(url,hm->uri.p,maxsize);
	url[maxsize]=0;
}

void HTTPData::getSourceIp(char *ip, int maxsize){
	mg_sock_addr_to_str(&(nc->sa), ip, maxsize, MG_SOCK_STRINGIFY_IP);
}

void HTTPData::getSourcePort(char *port, int maxsize){
	mg_sock_addr_to_str(&(nc->sa), port, maxsize, MG_SOCK_STRINGIFY_PORT);
}

void HTTPData::getSourceIpPort(char *ip, int maxsize){
	mg_sock_addr_to_str(&(nc->sa), ip, maxsize, MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
}

void HTTPData::setResponseCode(int code){
	hm->resp_code = code;
	hm->resp_status_msg.p = code2msg(code);
	hm->resp_status_msg.len = strlen(hm->resp_status_msg.p);
}

int HTTPData::getGET(const char *name, char *value, int maxsize){
	int i, j, found=-1;
	for(i=0;i<hm->query_string.len;i++){
		for(j=0;name[j]!=0 && i+j<hm->query_string.len;j++){
			if(name[j]!=hm->query_string.p[i+j]){
				break;
			}
		}
		if(name[j]==0 && i+j>=hm->query_string.len){
			found = i+j;
			break;
		}else if(name[j]==0 && hm->query_string.p[i+j]=='='){
			found = i+j+1;
			break;
		}
		while(i<hm->query_string.len && hm->query_string.p[i]!='&')i++;
	}
	if(found==-1)return 0;
	for(j=0;j<hm->query_string.len && hm->query_string.p[j+found]!='&';j++);
	mg_url_decode(hm->query_string.p+found, j, value, maxsize, 1);
	return 1;
}

int HTTPData::getPOST(const char *name, char *value, int maxsize){
	int i, j, found=-1;
	for(i=0;i<hm->message.len;i++){
		for(j=0;name[j]!=0 && i+j<hm->message.len;j++){
			if(name[j]!=hm->message.p[i+j]){
				break;
			}
		}
		if(name[j]==0 && i+j>=hm->message.len){
			found = i+j;
			break;
		}else if(name[j]==0 && hm->message.p[i+j]=='='){
			found = i+j+1;
			break;
		}
		while(i<hm->message.len && hm->message.p[i]!='&')i++;
	}
	if(found==-1)return 0;
	for(j=0;j<hm->message.len && hm->message.p[j+found]!='&';j++);
	mg_url_decode(hm->message.p+found, j, value, maxsize, 1);
	return 1;
}

const char* HTTPData::code2msg(int code){
	for(int i=0;codelist[i].code!=0;i++){
		if(codelist[i].code==code){
			return codelist[i].msg;
		}
	}
	return codelist[0].msg;
}

const char* HTTPData::ext2contentType(const char *name){
	int i = strlen(name), j;
	for(j=0;j<6 && name[i]!='.';j++,i--);
	if(j>=6 || j<1)return extlist[0].type;
	name += i+1;
	for(j=0;extlist[j].ext[0]!=0;j++){
		if(!strcmp(name,extlist[j].ext))return extlist[j].type;
	}
	return extlist[0].type;
}

void HTTPData::basicResponse(int code, const char *msg){
	mg_printf(nc, "HTTP/1.0 %d %s\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",
			code, code2msg(code), strlen(msg), msg);
	nc->flags |= MG_F_SEND_AND_CLOSE;
}

int HTTPData::printf(const char *fmt, ...) {
  int len;
  va_list ap;
  va_start(ap, fmt);
  len = mg_vprintf(nc, fmt, ap);
  va_end(ap);
  return len;
}

void HTTPData::send(char *ptr, int size){
	mg_send(nc, ptr, size);
}

const struct HTTPData::ExtList HTTPData::extlist[] = {
	{"bin",  "application/octet-stream"},
	{"aac",  "audio/aac"},
	{"avi",  "video/x-msvideo"},
	{"bz",   "application/x-bzip"},
	{"bz2",  "application/x-bzip2"},
	{"css",  "text/css"},
	{"csv",  "text/csv"},
	{"gif",  "image/gif"},
	{"htm",  "text/html"},
	{"html", "text/html"},
	{"ico",  "image/x-icon"},
	{"jpeg", "image/jpeg"},
	{"jpg",  "image/jpeg"},
	{"js",   "application/javascript"},
	{"json", "application/json"},
	{"mid",  "audio/midi"},
	{"midi", "audio/midi"},
	{"png",  "image/png"},
	{"pdf",  "application/pdf"},
	{"rar",  "application/x-rar-compressed"},
	{"sh",   "application/x-sh"},
	{"svg",  "image/svg+xml"},
	{"tar",  "application/x-tar"},
	{"tif",  "image/tiff"},
	{"tiff", "image/tiff"},
	{"txt",  "text/plain"},
	{"wav",  "audio/wav"},
	{"weba", "audio/webm"},
	{"webp", "image/webp"},
	{"xhtm", "application/xhtml+xml"},
	{"xml",  "application/xml"},
	{"zip",  "application/zip"},
	{"3gp",  "audio/3gpp"},
	{"3g2",  "audio/3gpp2"},
	{"7z",   "application/x-7z-compressed"},
	{"",     "application/octet-stream"},
};
const HTTPData::CodeList HTTPData::codelist[] = {
	{404, "Not Found"},
	{200, "OK"},
	{400, "Bad Request"},
	{100, "Continue"},
	{101, "Switching Protocols"},
	{102, "Processing"},
	{103, "Early Hints"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "Non-Authoritative Information"},
	{204, "No Content"},
	{205, "Reset Content"},
	{206, "Partial Content"},
	{207, "Multi-Status"},
	{208, "Already Reported"},
	{226, "IM Used"},
	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{304, "Not Modified"},
	{305, "Use Proxy"},
	{306, "Switch Proxy"},
	{307, "Temporary Redirect"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{407, "Proxy Authentication Required"},
	{408, "Request Timeout"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
	{412, "Precondition Failed"},
	{413, "Payload Too Large"},
	{414, "URI Too Long"},
	{415, "Unsupported Media Type"},
	{416, "Range Not Satisfiable"},
	{417, "Expectation Failed"},
	{418, "I'm a teapot"},
	{421, "Misdirected Request"},
	{422, "Unprocessable Entity"},
	{423, "Locked"},
	{424, "Failed Dependency"},
	{426, "Upgrade Required"},
	{428, "Precondition Required"},
	{429, "Too Many Requests"},
	{431, "Request Header Fields Too Large"},
	{451, "Unavailable For Legal Reasons"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Timeout"},
	{505, "HTTP Version Not Supported"},
	{506, "Variant Also Negotiates"},
	{507, "Insufficient Storage"},
	{508, "Loop Detected"},
	{510, "Not Extended"},
	{511, "Network Authentication Required"},
	{0,   ""},
};
