#include "StaticServeHandler.h"

#include "../HttpConstants.h"

extern const uint8_t indexHtmlFileStart[] asm(
    "_binary_web_portal_dist_index_html_start");
extern const uint8_t indexHtmlFileEnd[] asm(
    "_binary_web_portal_dist_index_html_end");
extern const uint8_t indexJsFileStart[] asm(
    "_binary_web_portal_dist_index_js_start");
extern const uint8_t indexJsFileEnd[] asm(
    "_binary_web_portal_dist_index_js_end");

void StaticServeHandler::handleHTML(AsyncWebServerRequest* request) {
  AsyncResponseStream* response = request->beginResponseStream(MimeType::Html);
  size_t dataLen = indexHtmlFileEnd - indexHtmlFileStart;
  response->write(indexHtmlFileStart, dataLen);
  request->send(response);
}

void StaticServeHandler::handleJS(AsyncWebServerRequest* request) {
  AsyncResponseStream* response =
      request->beginResponseStream(MimeType::Javascript);
  size_t dataLen = indexJsFileEnd - indexJsFileStart;
  response->write(indexJsFileStart, dataLen);
  request->send(response);
}
