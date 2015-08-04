#include <iostream>
#include <stdlib.h>
#include <assert.h>

#include "../spnkhttpcli.hpp"
#include "../spnkhttpmsg.hpp"
#include "../spnkporting.hpp"
#include "../spnksocket.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    spnk_initsock();//初始化spnetkit库

    SP_NKHttpRequest request;

    request.setURI("/html/index.html");
    request.setMethod("GET");
    request.setVersion("Http/1.1");
    request.addHeader("Connection", "keep-Alive");
    request.addHeader("Host", "127.0.0.1");

    SP_NKTcpSocket sp_socket("192.168.40.156", 80);

    SP_NKHttpResponse response;

    int ret = SP_NKHttpProtocol::get(&sp_socket, &request, &response);

    if(ret == 0){
        cout << "response:" << endl;
        cout << response.getVersion() <<"\n"<< response.getStatusCode()<<"\n" << response.getReasonPhrase() <<endl;

        cout << response.getHeaderCount()  << endl;

        for(int i = 0; i < response.getHeaderCount(); i++){
            const char *name = response.getHeaderName(i);
            const char *val  = response.getHeaderValue(i);
            cout << name << ": " << val << endl;
        }


        cout << response.getContentLength() << endl;


        if(NULL != response.getContent() && response.getContentLength() > 0 ){
            cout << (char *)response.getContent() << endl;
        }
    }else{
        
        cout << "Http request failed!\n";
    }

    return 0;
}
