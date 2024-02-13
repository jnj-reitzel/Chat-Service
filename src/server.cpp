//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2024 Julian Reitzel
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <vector>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcp_connection
    : public std::enable_shared_from_this<tcp_connection> 
{
public:
    typedef std::shared_ptr<tcp_connection> pointer;
    static inline int count = 0;

    static pointer New(boost::asio::io_context& io_context)
    {
        return pointer(new tcp_connection(io_context));
    }
    void start() {
        do_read();
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    int id()
    {
        return id_;
    }

private:

    tcp_connection(boost::asio::io_context& io_context)
        :   socket_(io_context), 
            id_(count++) 
        {}

    void do_read() {
        auto self(shared_from_this());
        boost::asio::async_read_until(socket_, input_buffer_, '\n',
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string message(boost::asio::buffers_begin(input_buffer_.data()), boost::asio::buffers_begin(input_buffer_.data()) + length);
                    input_buffer_.consume(length); // Remove consumed input from the streambuf
                    std::cout << "User " << id_ << " says: " << message;
                    do_read(); // Continue reading
                } else {
                    std::cerr << "Error reading message from user " << id_ << ": " << ec.message() << std::endl;
                }
            });
    }

    tcp::socket socket_;
    int id_;
    boost::asio::streambuf input_buffer_;
};


class chatroom {
public:
    chatroom(short port)
        :   io_context_(),
            acceptor_(io_context_, tcp::endpoint(tcp::v4(), port))
    {
        do_accept();
    }

    void run()  {
        io_context_.run();
    }

private:
    void do_accept() {
        tcp_connection::pointer new_connection = tcp_connection::New(io_context_);

        acceptor_.async_accept(new_connection->socket(),
            [this, new_connection](boost::system::error_code ec) {
            // [this](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "New connection "  << std::endl;
                    // auto session = std::make_shared<tcp_connection>(std::move(socket), next_id_++);
                    sessions_.push_back(new_connection);
                    new_connection->start();
                }
                do_accept();
        });
    }

    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::vector<tcp_connection::pointer> sessions_;
};


int main() {
    try {
        chatroom server(8080);
        server.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
