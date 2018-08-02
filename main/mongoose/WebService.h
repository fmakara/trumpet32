/*
 * WebService.h
 *
 *  Created on: 21 de mai de 2018
 *      Author: makara
 */

#ifndef MAIN_MONGOOSE_WEBSERVICE_H_
#define MAIN_MONGOOSE_WEBSERVICE_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mongoose.h"
#include "HTTPData.h"
#include <map>

class WebService {
public:
  WebService(const char *port);
  void addCgi(const char *page, void (*cb)(HTTPData*), const char *type=NULL);
  void addSpiffs(const char *page, const char *path);
  static int w_strcmp(const char *wild, const char *s2, int size=-1);
  static void w_strfix(char *to, const char *wildFrom, const char *wildTo, const char *from, int size=-1);
protected:
  enum PageType {
    TYPE_CGI,
    TYPE_SPIFFS,
  };
  struct Page {
    enum PageType type;
    char page[32];
    char internalPath[32];
    void (*cb)(HTTPData*);
    struct Page *next;
  };
  std::map<sock_t,FILE*> sendingData;
  struct Page *list;
  struct mg_mgr mgr;
  struct mg_connection *nc;
  TaskHandle_t handle;
  static void mg_ev_handler(struct mg_connection *nc, int ev, void *p);
  static void task(void *arg);
};

#endif /* MAIN_MONGOOSE_WEBSERVICE_H_ */
