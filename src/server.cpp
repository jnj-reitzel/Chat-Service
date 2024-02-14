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

    static pointer New(boost::asio::io_context& io_context, int id)
    {
        return pointer(new connection(io_context, id));
    }
    tcp::socket& socket()
    {
        return socket_;
    }

    int id() const 
    {
        return id_;
    }

    boost::asio::streambuf& input_buffer() 
    {
        return input_buffer_;
    }

    const std::string& username() const 
    {
        return username_;
    }

    void set_username(const std::string& username)
    {
        username_ = username;
    }

    std::string to_string() const 
    {
        return username() + " (ID:" +std::to_string(id()) + ")";
    }

private:

    connection(boost::asio::io_context& io_context, int id)
        :   socket_(io_context), 
            id_(id),
            username_("User" + std::to_string(id)) 
        {}


    boost::asio::streambuf input_buffer_{};
    tcp::socket socket_;
    int id_;
    std::string username_;
};

class chatroom {
public:
    chatroom(const std::string& ip, const std::string& port)
        :   
            acceptor_(io_context_, *(tcp::resolver(io_context_).resolve(ip, port).begin())),
            next_id(0)
    {
        tcp::acceptor::endpoint_type endpoint = acceptor_.local_endpoint();
        std::cout << "Open server at "
            << endpoint.address().to_string() << ":" 
            << endpoint.port() << std::endl;
        accept_new_connection();
    }

    void run()  {
        io_context_.run();
    }

private:
    void accept_new_connection() {
        connection::pointer new_connection = connection::New(io_context_, next_id++);

        acceptor_.async_accept(new_connection->socket(),
            [this, new_connection](boost::system::error_code ec) {
                if (!ec) {
                    receive_and_set_username(new_connection);
                    std::cout << "New connection: " << new_connection->to_string()  << std::endl;
                    connections.push_back(new_connection);
                    read_and_broadcast(new_connection);
                }
                accept_new_connection();
        });
    }

    void receive_and_set_username(connection::pointer new_connection)
    {
        boost::asio::read_until(new_connection->socket(), new_connection->input_buffer(), '\n');

        // Extract the received username from the buffer
        std::istream is(&new_connection->input_buffer());
        std::string username;
        std::getline(is, username);
        if (!username.empty()) 
        {
            new_connection->set_username(username);
        }
    }

    void read_and_broadcast(connection::pointer existing_connection) {
        boost::asio::async_read_until(existing_connection->socket(), existing_connection->input_buffer(), '\n',
            [this, existing_connection](boost::system::error_code ec, std::size_t length) {
                boost::asio::streambuf& buffer = existing_connection->input_buffer();
                if (!ec) {
                    std::string message(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_begin(buffer.data()) + length);
                    existing_connection->input_buffer().consume(length); // Remove consumed input from the streambuf
                    std::cout << existing_connection->to_string() << " writes: " << message;
                    broadcast(message, existing_connection);
                    read_and_broadcast(existing_connection); // Continue reading
                } else if (ec == boost::asio::error::eof) {
                    // Handle end-of-file condition (EOF)
                    std::cerr << "Closed connection: " << existing_connection->to_string() << std::endl;
                } else {
                    // Handle other errors
                    std::cerr << "Error reading message: " << ec.message() 
                        << "; from: " << existing_connection->to_string() << std::endl;
            }
            });
    }

    void broadcast(const std::string& message, connection::pointer origin_connection)
    {;
        std::string mod_message = origin_connection->username() + " > " + message;
        for (const auto& con : connections) 
        {
            if (con != origin_connection) 
            {
                boost::asio::async_write(con->socket(), boost::asio::buffer(mod_message),
                [](const boost::system::error_code& error, std::size_t) {
                    if (error) {
                        std::cerr << "Error sending message: " << error.message() << std::endl;
                    }
                });
            }
        }
    }

    boost::asio::io_context io_context_{};
    std::vector<connection::pointer> connections{};
    tcp::acceptor acceptor_;
    int next_id;
};

int main(int argc, char* argv[]) 
{
    std::string ip, port, username;

    if (argc == 3) {
        // ip and port provided as command-line arguments
        ip = argv[1];
        port = argv[2];
    }
    else
    {
        ip = "localhost";
        port = "8080";
    }

    try {
        chatroom chat1(ip, port);
        chat1.run();
    } catch (const std::exception& e) { 
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
