#include "http.h"
#include "image.h"
#include <vector>
#include <string>
#include <iostream>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "bitmap.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::to_string;
using std::string;


extern vector<individual> population;
extern bool quit;
extern int min_fitness;
extern int max_fitness;
extern int initial_fitness;

sockaddr_in get_ip_address(const char* hostname, int port){
	sockaddr_in ipa;
	ipa.sin_family = AF_INET;
	ipa.sin_port = htons(port);

	auto host = gethostbyname(hostname);
	if(!host){
        cout << "Issue resolving host!" << endl;
		exit(1);
	}

	auto addr = host->h_addr_list[0];
	memcpy(&ipa.sin_addr.s_addr, addr, sizeof addr);

	return ipa;
}

void run_server(int port) {

    auto tcp = getprotobyname("tcp");
    auto fd = socket(AF_INET, SOCK_STREAM, tcp->p_proto);

    if (fd == -1) {
        cout << "Error: Opening socket!" << endl;
        exit(1);
    }

    auto ip = get_ip_address("0.0.0.0", port);


    if(bind(fd, (sockaddr*)&ip, sizeof(ip)) == -1) {
        cout << "Error: Failed to bind!" << endl;
        exit(1);
    }

    if (listen(fd, 1) == -1) {
        cout << "Error: Failed to listen!" << endl;
    }

    cout << "Listening on port " << port << "!" << endl;

    while(!quit) {

        auto cd = accept(fd, nullptr, nullptr);

        auto& most_fittest = population[0];

        char buffer[1024];
        memset(buffer, 0, 1024);
        read(cd, buffer, 1024);

        string html_request(buffer);

        string http_response;

        if (html_request.find("/data") != -1)
        {
            bitmap bmp(most_fittest.data(), most_fittest.get_width(), most_fittest.get_height());

            auto raw_bitmap_data = bmp.make();

            http_response =  //Create the HTTP response
            string("HTTP/1.1 200 OK\r\n") +
            "Content-Length: " + to_string(raw_bitmap_data.size()) + "\r\n" +
            "Content-Type: image/x-windows-bmp\r\n\r\n" +
            raw_bitmap_data;
        }
        else 
        {
            auto message =  // TODO HTML Builder class
            "<html><body>Initial Fitness: " 
            + to_string(initial_fitness) 
            + "<br>Minimum Fitness: " 
            + to_string(min_fitness) + 
            "<br>Maximum Fitness:" + to_string(max_fitness) 
            + "<br>Current Fitness: "  + to_string(most_fittest.get_fitness()) + 
            "<br>Current Frame:</br><img src='/data' />" +
            "</body></html>";

            http_response =  //Create the HTTP response
            string("HTTP/1.1 200 OK\r\n") +
            "Content-Length: " + to_string(message.size()) + "\r\n" +
            "Content-Type: text/html\r\n\r\n" +
            message;

        }
       
        auto status = send(cd, http_response.c_str(), http_response.size(), MSG_NOSIGNAL); // Send the HTTP response to the client

        shutdown(cd, SHUT_RDWR);

    }

}
