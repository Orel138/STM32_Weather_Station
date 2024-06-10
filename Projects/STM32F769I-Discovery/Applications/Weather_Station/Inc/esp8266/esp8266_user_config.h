/*
 * Copyright (c) 2024 Orel138
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ESP8266_USER_CONFIG_H
#define __ESP8266_USER_CONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/*! Define constants for API keys and endpoints */
#define OPENWEATHERMAP_API_KEY "NULL"
#define OPENSENSEMAP_API_KEY "NULL"

#define OPENSENSEMAP_BOX "NULL"
#define OPENSENSEMAP_SENSOR1 "NULL"

#define OPENWEATHERMAP_HOST "api.openweathermap.org"
#define OPENWEATHERMAP_PORT 80

#define OPENSENSEMAP_HOST "api.opensensemap.org"
#define OPENSENSEMAP_PORT 443

#define OPENWEATHERMAP_HTML_SUCCESS_STRING "HTTP/1.1 200 OK"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
 
#ifdef __cplusplus
}
#endif

#endif /* __ESP8266_USER_CONFIG_H */
