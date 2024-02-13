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

class connection
    : public std::enable_shared_from_this<connection> 
{
public:
    typedef std::shared_ptr<connection> pointer;
    static inline int count = 0;

    static pointer New(boost::asio::io_context& io_context)
    {
        return pointer(new connection(io_context));
    }
    // void start() {
    //     do_read();
    // }

    tcp::socket& socket()
    {
        return socket_;
    }

    int id()
    {
        return id_;
    }

    boost::asio::streambuf& input_buffer() 
    {
        return input_buffer_;
    }

private:

    connection(boost::asio::io_context& io_context)
        :   socket_(io_context), 
            id_(count++) 
        {}


    tcp::socket socket_;
    int id_;
    std::string username_;
    boost::asio::streambuf input_buffer_;
};


class chatroom {
public:
    chatroom(short port)
        :   io_context_(),
            acceptor_(io_context_, tcp::endpoint(tcp::v4(), port))
    {
        accept_new_connection();
    }

    void run()  {
        io_context_.run();
    }

private:
    void accept_new_connection() {
        connection::pointer new_connection = connection::New(io_context_);

        acceptor_.async_accept(new_connection->socket(),
            [this, new_connection](boost::system::error_code ec) {
            // [this](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "New connection "  << std::endl;
                    // auto session = std::make_shared<connection>(std::move(socket), next_id_++);
                    connections.push_back(new_connection);
                    read_and_broadcast(new_connection);
                }
                accept_new_connection();
        });
    }
    void read_and_broadcast(connection::pointer existing_connection) {
        // auto self(shared_from_this());
        boost::asio::async_read_until(existing_connection->socket(), existing_connection->input_buffer(), '\n',
            [this, existing_connection](boost::system::error_code ec, std::size_t length) {
                boost::asio::streambuf& buffer = existing_connection->input_buffer();
                if (!ec) {
                    std::string message(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + length);
                    existing_connection->input_buffer().consume(length); // Remove consumed input from the streambuf
                    std::cout << "User "<< " says: " << message;
                    broadcast(message, existing_connection);
                    read_and_broadcast(existing_connection); // Continue reading
                } else {
                    std::cerr << "Error reading message from user " << ": " << ec.message() << std::endl;
                }
            });
    }

    void broadcast(const std::string& message, connection::pointer origin_connection)
    {;
        for (auto& con : connections) 
        {
            if (con != origin_connection) 
            {
                boost::asio::async_write(con->socket(), boost::asio::buffer(message),
                [](const boost::system::error_code& error, std::size_t bytes_transferred) {
                    if (error) {
                        std::cerr << "Error sending message: " << error.message() << std::endl;
                    }
                });
            }
        }
    }

    boost::asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::vector<connection::pointer> connections;
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
