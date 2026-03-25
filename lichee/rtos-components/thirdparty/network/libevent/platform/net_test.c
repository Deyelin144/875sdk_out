/**
 * @file net_test.c
 * @author hubertxxu (hubertxxu@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-09-11
 * 
 * @copyright
 * Tencent is pleased to support the open source community by making IoT Hub available. 
 * Copyright(C) 2021 - 2026 THL A29 Limited, a Tencent company.All rights reserved.
 * Licensed under the MIT License(the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "tc_iot_hal.h"
#include "tc_iot_log.h"

static char *DigiCert_Global_Root_CA =  {
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIEMjCCAxqgAwIBAgIBATANBgkqhkiG9w0BAQUFADB7MQswCQYDVQQGEwJHQjEb\r\n"
    "MBkGA1UECAwSR3JlYXRlciBNYW5jaGVzdGVyMRAwDgYDVQQHDAdTYWxmb3JkMRow\r\n"
    "GAYDVQQKDBFDb21vZG8gQ0EgTGltaXRlZDEhMB8GA1UEAwwYQUFBIENlcnRpZmlj\r\n"
    "YXRlIFNlcnZpY2VzMB4XDTA0MDEwMTAwMDAwMFoXDTI4MTIzMTIzNTk1OVowezEL\r\n"
    "MAkGA1UEBhMCR0IxGzAZBgNVBAgMEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\r\n"
    "BwwHU2FsZm9yZDEaMBgGA1UECgwRQ29tb2RvIENBIExpbWl0ZWQxITAfBgNVBAMM\r\n"
    "GEFBQSBDZXJ0aWZpY2F0ZSBTZXJ2aWNlczCCASIwDQYJKoZIhvcNAQEBBQADggEP\r\n"
    "ADCCAQoCggEBAL5AnfRu4ep2hxxNRUSOvkbIgwadwSr+GB+O5AL686tdUIoWMQua\r\n"
    "BtDFcCLNSS1UY8y2bmhGC1Pqy0wkwLxyTurxFa70VJoSCsN6sjNg4tqJVfMiWPPe\r\n"
    "3M/vg4aijJRPn2jymJBGhCfHdr/jzDUsi14HZGWCwEiwqJH5YZ92IFCokcdmtet4\r\n"
    "YgNW8IoaE+oxox6gmf049vYnMlhvB/VruPsUK6+3qszWY19zjNoFmag4qMsXeDZR\r\n"
    "rOme9Hg6jc8P2ULimAyrL58OAd7vn5lJ8S3frHRNG5i1R8XlKdH5kBjHYpy+g8cm\r\n"
    "ez6KJcfA3Z3mNWgQIJ2P2N7Sw4ScDV7oL8kCAwEAAaOBwDCBvTAdBgNVHQ4EFgQU\r\n"
    "oBEKIz6W8Qfs4q8p74Klf9AwpLQwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQF\r\n"
    "MAMBAf8wewYDVR0fBHQwcjA4oDagNIYyaHR0cDovL2NybC5jb21vZG9jYS5jb20v\r\n"
    "QUFBQ2VydGlmaWNhdGVTZXJ2aWNlcy5jcmwwNqA0oDKGMGh0dHA6Ly9jcmwuY29t\r\n"
    "b2RvLm5ldC9BQUFDZXJ0aWZpY2F0ZVNlcnZpY2VzLmNybDANBgkqhkiG9w0BAQUF\r\n"
    "AAOCAQEACFb8AvCb6P+k+tZ7xkSAzk/ExfYAWMymtrwUSWgEdujm7l3sAg9g1o1Q\r\n"
    "GE8mTgHj5rCl7r+8dFRBv/38ErjHT1r0iWAFf2C3BUrz9vHCv8S5dIa2LX1rzNLz\r\n"
    "Rt0vxuBqw8M0Ayx9lt1awg6nCpnBBYurDC/zXDrPbDdVCYfeU0BsWO/8tqtlbgT2\r\n"
    "G9w84FoVxp7Z8VlIMCFlA2zs6SFz7JsDoeA3raAVGI/6ugLOpyypEBMs1OUIJqsi\r\n"
    "l2D4kF501KKaU73yqWjgom7C12yxow+ev+to51byrvLjKzg6CYG1a4XXvi3tPxq3\r\n"
    "smPi9WIsgtRqAEFQ8TmDn5XpNpaYbg==\r\n"
    "-----END CERTIFICATE-----\r\n"};

int tls_test(void)
{
    int rc = 0;
    HAL_Printf("\r\n============================>>> tls test begin <<<=========================>>>\r\n\r\n");
    //const char *host = "httpbin.org";
    const char *host = "blog.csdn.net";
    int port = 443;

    uintptr_t tls_handle = 0;

    TLSConnectParams params;
    memset(&params, 0, sizeof(params));
    params.ca_crt = DigiCert_Global_Root_CA;
    params.ca_crt_len = strlen(DigiCert_Global_Root_CA);
    params.timeout_ms = 10000;
    // connect
    tls_handle =   HAL_TLS_Connect(&params, host, port);
                
    if(!tls_handle){
        Log_e("%s connect failed.", host);
        return -1;
    }
    Log_i("%s connect success, handle : %p", host, tls_handle);

    // sendmsg
    char write_buf[1024];
    size_t real_write = 0;
   // int write_len = HAL_Snprintf(write_buf, 1024, "POST /post HTTP/1.1\r\nHost: httpbin.org\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: 11\r\nConnection: close\r\n\r\nHello=World");
   int write_len = HAL_Snprintf(write_buf, 1024, "GET /HumorRat/article/details/6013064 HTTP/1.1\r\nHost: blog.csdn.net\r\nConnection: close\r\n\r\n");

   rc = HAL_TLS_Write(tls_handle,(unsigned char *)write_buf, write_len,10000,&real_write);   
    if(rc){
        Log_e("HAL_TLS_Write failed.");
        goto exit;
    }

    // recvmsg
    char recv_buf[1024] = {0};
    memset(recv_buf, 0, 1024);
    size_t recv_len = 0;
    rc = HAL_TLS_Read(tls_handle, (unsigned char *)recv_buf, 1024, 10000, &recv_len);
    if(rc){
        Log_e("HAL_TLS_Read failed. rc = %d",rc);
        goto exit;
    }
    Log_i("recv(%d) : %s",recv_len, recv_buf);

exit:
    HAL_TLS_Disconnect(tls_handle);
    HAL_Printf("\r\n============================>>> tls test end <<<=========================>>>\r\n\r\n");
    return 0; // ignore return
}












