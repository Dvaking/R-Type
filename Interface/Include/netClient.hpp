/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** netClient
*/

#pragma once
#include "netCommon.hpp"
#include "netConnection.hpp"
#include "netThreadSafeQueue.hpp"

namespace olc {
namespace net {
template <typename T> class ClientInterface {
  public:
    ClientInterface() {}

    virtual ~ClientInterface() { Disconnect(); }

  public:
    bool Connect(const std::string &host, const uint16_t port)
    {
        try {
            asio::ip::tcp::resolver resolver(m_context);
            asio::ip::tcp::resolver::results_type endpoints =
                resolver.resolve(host, std::to_string(port));

            m_connection = std::make_unique<Connection<T>>(Connection<T>::owner::client, m_context,
                asio::ip::tcp::socket(m_context), m_qMessagesIn);

            m_connection->ConnectToServer(endpoints);

            thrContext = std::thread([this]() { m_context.run(); });
        } catch (std::exception &e) {
            std::cerr << "Client Exception: " << e.what() << "\n";
            return false;
        }
        return true;
    }

    void Disconnect()
    {
        if (IsConnected()) {
            m_connection->Disconnect();
        }

        m_context.stop();
        if (thrContext.joinable())
            thrContext.join();

        m_connection.release();
    }

    bool IsConnected()
    {
        if (m_connection)
            return m_connection->IsConnected();
        else
            return false;
    }

  public:
    void Send(const message<T> &msg)
    {
        if (IsConnected())
            m_connection->Send(msg);
    }

    ThreadSafeQueue<owned_message<T>> &Incoming() { return m_qMessagesIn; }

  protected:
    asio::io_context m_context;
    std::thread thrContext;
    std::unique_ptr<Connection<T>> m_connection;

  private:
    ThreadSafeQueue<owned_message<T>> m_qMessagesIn;
};
} // namespace net
} // namespace olc