//
// client.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2024 Julian Reitzel
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace client {

    class user {
    public:
        user(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints)
            : socket_(io_context), stdin_stream_(io_context, ::dup(STDIN_FILENO)) {
            do_connect(endpoints);
        }

        void do_connect(const tcp::resolver::results_type& endpoints) {
            boost::asio::async_connect(socket_, endpoints,
                [this](boost::system::error_code ec, tcp::endpoint) {
                    if (!ec) {
                        std::cout << "Connected to server" << std::endl;
                        do_read_input();
                        do_receive_message();
                    } else {
                        std::cerr << "Error connecting to server: " << ec.message() << std::endl;
                    }
                });
        }

        void do_read_input() {
            boost::asio::async_read_until(stdin_stream_, input_, '\n',
                [this](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::string message(boost::asio::buffers_begin(input_.data()), boost::asio::buffers_begin(input_.data()) + length);
                        async_send_message(message);
                        input_.consume(length); // Remove consumed input from the streambuf
                        do_read_input(); // Continue reading input
                    } else {
                        std::cerr << "Error reading input: " << ec.message() << std::endl;
                    }
                });
        }

        void async_send_message(const std::string& message) {
            boost::asio::async_write(socket_, boost::asio::buffer(message),
                [this](const boost::system::error_code& error, std::size_t bytes_transferred) {
                    if (error) {
                        std::cerr << "Error sending message: " << error.message() << std::endl;
                    }
                });
        }

        void do_receive_message() {
            boost::asio::async_read_until(socket_, response_, '\n',
                [this](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::string message(boost::asio::buffers_begin(response_.data()), boost::asio::buffers_begin(response_.data()) + length);
                        std::cout << "Received: " << message << std::endl;
                        response_.consume(length); // Remove consumed input from the streambuf
                        do_receive_message(); // Continue reading messages
                    } else {
                        std::cerr << "Error receiving message: " << ec.message() << std::endl;
                    }
                });
        }

    private:
        tcp::socket socket_;
        boost::asio::posix::stream_descriptor stdin_stream_;
        boost::asio::streambuf input_;
        boost::asio::streambuf response_;
    };

}

int main() {
    boost::asio::io_context io_context;
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve("localhost", "8080");
    
    client::user user1(io_context, endpoints);

    io_context.run();

    return 0;
}
