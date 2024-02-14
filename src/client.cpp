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
#include <filesystem>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;


class user_connection {
    public:
        user_connection(const std::string& ip, const std::string& port, const std::string& username)
            :   
            endpoints_(tcp::resolver(io_context_).resolve(ip, port)),
            username_(username) 
        {}

        void connect_to_chatroom() {
            boost::system::error_code ec;
            boost::asio::connect(socket_, endpoints_, ec);
            if (!ec) {
                std::cout << "Connected to the server at "
                    << endpoints_.begin()->endpoint().address().to_string() << ":" 
                    << endpoints_.begin()->endpoint().port() << std::endl
                    << "Write your messages and press ENTER." << std::endl;
                send_username();
                start_recurring_events();
            } else {
                std::cerr << "Error connecting to server: " << ec.message() << std::endl;
            }
            io_context_.run();
        }
        const std::string& username() const
        {
            return username_;
        }

    private:

        void start_recurring_events()
        {
            read_and_send_stdin();
            receive_and_print_message();
        }

        void send_username() 
        {
            send_message(username() + "\n");
            input_.consume(username().size());
        }

        void send_message(const std::string& message) 
        {
            boost::asio::async_write(socket_, boost::asio::buffer(message),
                [this](const boost::system::error_code& error, std::size_t) {
                    if (error) {
                        std::cerr << "Error sending message: " << error.message() << std::endl;
                    }
                });
        }
        
        void read_and_send_stdin()
        {   
            boost::asio::async_read_until(stdin_stream_, input_, '\n',
                [this](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::string message(boost::asio::buffers_begin(input_.data()), boost::asio::buffers_begin(input_.data()) + length);
                        send_message(message);
                        input_.consume(length); // Remove consumed input from the streambuf
                        read_and_send_stdin(); // Continue reading input
                    } else {
                        std::cerr << "Error reading input: " << ec.message() << std::endl;
                    }
                });
        }

        void receive_and_print_message() 
        {
            boost::asio::async_read_until(socket_, response_, '\n',
                [this](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        std::string message(boost::asio::buffers_begin(response_.data()), boost::asio::buffers_begin(response_.data()) + length);
                        std::cout << message;
                        response_.consume(length); // Remove consumed input from the streambuf
                        receive_and_print_message(); // Continue reading messages
                    } else {
                        std::cerr << "Error receiving message: " << ec.message() << std::endl;
                    }
                });
        }

        boost::asio::io_context io_context_{};
        tcp::socket socket_{io_context_};
        boost::asio::posix::stream_descriptor stdin_stream_{io_context_, ::dup(STDIN_FILENO)};
        boost::asio::streambuf input_{};
        boost::asio::streambuf response_{};
        tcp::resolver::results_type endpoints_;
        const std::string username_;

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

    std::cout << "Enter your username: ";
    std::getline(std::cin, username);

    user_connection user1(ip, port, username);
    user1.connect_to_chatroom();

    return 0;
}
