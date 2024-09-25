#pragma once
#include "netCommon.hpp"
#include "netConnection.hpp"
#include "netMessage.hpp"
#include "netThreadSafeQueue.hpp"

namespace olc {
namespace net {
template <typename T> class ClientInterface {
    ClientInterface() : m_socket(m_context) {}
    virtual ~ClientInterface() { Disconnect(); }

  public:
    bool Connect(const std::string &host, const uint16_t port)
    {
        try {
            m_connection = std::make_unique<connection<T>>(); // TODO

            asio::ip::tcp::resolver resolver(m_context);
            asio::ip::tcp::resolver::results_type endpoints =
                resolver.resolve(host, std::to_string(port));
            m_connection->ConnectToServer(endpoints);
            thrContext = std::thread([this]() { m_context.run(); });
        } catch (std::exception &e) {
            std::cerr << "Client Exception: " << e.what() << std::endl;
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
        if (thrContext.joinable()) {
            thrContext.join();
        }
        m_connection.release();
    }
    bool IsConnected()
    {
        if (m_connection) {
            return m_connection->IsConnected();
        } else {
            return false;
        }
        bool IsConnected()
        {
            if (m_connection) {
                return m_connection->IsConnected();
            } else {
                return false;
            }
        }
        ThreadSafeQueue<owned_message<T>> &Incoming() { return m_qMessagesIn; }

      protected:
        asio::io_context m_context;
        std::thread thrContext;
        asio::ip::tcp::socket m_socket;
        std::unique_ptr<connection<T>> m_connection;

      private:
        ThreadSafeQueue<owned_message<T>> m_qMessagesIn;
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
    asio::ip::tcp::socket m_socket;
    std::unique_ptr<connection<T>> m_connection;

  private:
    ThreadSafeQueue<owned_message<T>> m_qMessagesIn;
};
} // namespace net
} // namespace olc
