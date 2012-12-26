/*
 Copyright 2012 Roland Littwin (repetier) repetierdev@gmail.com
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */


#include <iostream>
#include "mongoose.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "printer.h"
#include "global_config.h"
#include "WebserverAPI.h"

namespace po = boost::program_options;
using namespace std;

static const char *html_form =
"<html><body>POST example."
"<form method=\"POST\" action=\"/printer/handle_post_request\">"
"Input 1: <input type=\"text\" name=\"input_1\" /> <br/>"
"Input 2: <input type=\"text\" name=\"input_2\" /> <br/>"
"<input type=\"submit\" />"
"</form></body></html>";

static void *callback(enum mg_event event,
                      struct mg_connection *conn) {
    const struct mg_request_info *ri = mg_get_request_info(conn);
    
    if (event == MG_NEW_REQUEST) {
        if(strncmp(ri->uri,"/printer/",9)!=0) return repetier::HandlePagerequest(conn);
        repetier::HandleWebrequest(conn);
        /*
        if (!strcmp(ri->uri, "/printer/handle_post_request")) {
            // User has submitted a form, show submitted data and a variable value
            char post_data[1024],
            input1[sizeof(post_data)], input2[sizeof(post_data)];
            int post_data_len;
            
            // Read POST data
            post_data_len = mg_read(conn, post_data, sizeof(post_data));
            
            // Parse form data. input1 and input2 are guaranteed to be NUL-terminated
            mg_get_var(post_data, post_data_len, "input_1", input1, sizeof(input1));
            mg_get_var(post_data, post_data_len, "input_2", input2, sizeof(input2));
            
            mg_printf(conn, "HTTP/1.0 200 OK\r\n"
                      "Content-Type: text/plain\r\n\r\n"
                      "Submitted data: [%.*s]\n"
                      "Submitted data length: %d bytes\n"
                      "input_1: [%s]\n"
                      "input_2: [%s]\n",
                      post_data_len, post_data, post_data_len, input1, input2);
        } else {
            // Show HTML form.
            mg_printf(conn, "HTTP/1.0 200 OK\r\n"
                      "Content-Length: %d\r\n"
                      "Content-Type: text/html\r\n\r\n%s",
                      (int) strlen(html_form), html_form);
        }*/
        // Mark as processed
        return (void*)"";
    } else {
        return NULL; //repetier::HandlePagerequest(conn);;
    }
}

int main(int argc, const char * argv[])
{
    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "produce help message")
    ("config,c", po::value<string>(), "Configuration file")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    string confFile;
    if(vm.count("config")) {
        confFile = vm["config"].as<string>();
    }
    if(confFile.length() == 0) confFile = string("/etc/repetier-server.conf");
    boost::filesystem::path cf(confFile);
    if(!boost::filesystem::exists(cf)) {
        cerr << "Configuration file not found at " << confFile << endl;
        cerr << "Please use config option with correct path" << endl;
        cerr << desc << endl;
        return 2;
    }
    gconfig = new GlobalConfig(confFile); // Read global configuration
    gconfig->readPrinterConfigs();
    gconfig->startPrinterThreads();
    struct mg_context *ctx;
    const char *options[] = {"document_root", "/Users/littwin/Documents/Projekte/Repetier-Server/Repetier-Server/www/","listening_ports", gconfig->getPorts().c_str(), NULL};
    
    ctx = mg_start(&callback, NULL, options);
    //getchar();  // Wait until user hits "enter"
    while(true) {
        if(getchar()=='x') break;
    }
    mg_stop(ctx);
    cout << "Closing server" << endl;
    gconfig->stopPrinterThreads();
    return 0;
}
