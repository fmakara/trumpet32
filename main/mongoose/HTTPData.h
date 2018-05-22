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
	struct http_message *hm;
	struct mg_connection *nc;
	void return_404(const char *msg);
private:
};

#endif /* MAIN_MONGOOSE_HTTPDATA_H_ */
