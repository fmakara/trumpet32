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


void HTTPData::return_404(const char *msg){
	mg_printf(nc, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",strlen(msg),msg);
	nc->flags |= MG_F_SEND_AND_CLOSE;
}
