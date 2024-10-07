/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** netServer
*/

#pragma once

#include <net_server_interface.hpp>

namespace r_type {
namespace net {
/**
 * @brief NetServerAbstract class
 *
 * @tparam T
 */
template <typename T> class NetServerAbstract : virtual public r_type::net::NetServerInterface<T> {
  public:
    /**
     * @brief Construct a new Server Interface object
     *
     * @param port
     */
    NetServerAbstract(uint16_t port) : r_type::net::NetServerInterface<T>(port) {}

    /**
     * @brief Destroy the Server Interface object
     *
     */
    ~NetServerAbstract() { Stop(); }
    /**
     * @brief Start the server
     *
     * @return true
     * @return false
     */
    bool Start()
    {
        try {
            WaitForClientMessage();

            m_threadContext = std::thread([this]() { m_asioContext.run(); });
        } catch (std::exception &e) {
            std::cerr << "[SERVER] Exception: " << e.what() << "\n";
            return false;
        }

        std::cout << "[SERVER] Started!\n";
        return true;
    }

    /**
     * @brief Stop the server
     *
     */
    void Stop()
    {
        m_asioContext.stop();

        if (m_threadContext.joinable())
            m_threadContext.join();

        std::cout << "[SERVER] Stopped!\n";
    }

    /**
     * @brief wait for client connection
     *
     */
    void WaitForClientMessage()
    {
        m_asioSocket.async_receive_from(asio::buffer(m_tempBuffer.data(), m_tempBuffer.size()),
            m_clientEndpoint, [this](std::error_code ec, std::size_t bytes_recvd) {
                if (m_clientEndpoint.protocol() != asio::ip::udp::v4())
                    return WaitForClientMessage();
                if (!ec) {
                    std::cout << "[SERVER] New Connection: "
                              << m_clientEndpoint.address().to_string() << ":"
                              << m_clientEndpoint.port() << std::endl;
                    // create client socket
                    asio::ip::udp::socket newClientSocket(m_asioContext);
                    newClientSocket.open(m_clientEndpoint.protocol());
                    newClientSocket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), 0));

                    std::cout << newClientSocket.local_endpoint().address().to_string() << ":"
                              << newClientSocket.local_endpoint().port() << std::endl;

                    std::shared_ptr<Connection<T>> newConn =
                        std::make_shared<Connection<T>>(Connection<T>::owner::server,
                            m_asioContext, std::move(newClientSocket), m_qMessagesIn);

                    if (OnClientConnect(newConn)) {
                        m_deqConnections.push_back(std::move(newConn));
                        m_deqConnections.back()->ConnectToClient(this, nIDCounter++);
                        std::cout << "[" << m_deqConnections.back()->GetID()
                                  << "] Connection Approved\n";
                    } else {
                        std::cout << "[-----] Connection Denied\n";
                    }
                } else {
                    std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
                }
            });
    }

    /**
     * @brief send message message to client
     *
     * @param client
     * @param msg
     */
    void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T> &msg)
    {
        if (client && client->IsConnected()) {
            client->Send(msg);
        } else {
            OnClientDisconnect(client);

            client.reset();

            m_deqConnections.erase(
                std::remove(m_deqConnections.begin(), m_deqConnections.end(), client),
                m_deqConnections.end());
        }
    }

    /**
     * @brief message all clients
     *
     * @param msg
     * @param pIgnoreClient
     */
    void MessageAllClients(
        const Message<T> &msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr)
    {
        bool bInvalidClientExists = false;

        for (auto &client : m_deqConnections) {
            if (client && client->IsConnected()) {
                if (client != pIgnoreClient)
                    client->Send(msg);
            } else {
                OnClientDisconnect(client);
                client.reset();

                bInvalidClientExists = true;
            }
        }

        if (bInvalidClientExists)
            m_deqConnections.erase(
                std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr),
                m_deqConnections.end());
    }

    /**
     * @brief update server
     *
     * @param nMaxMessages
     * @param bWait
     */
    void Update(size_t nMaxMessages = -1, bool bWait = false)
    {
        if (bWait)
            m_qMessagesIn.wait();

        size_t nMessageCount = 0;
        while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty()) {
            auto msg = m_qMessagesIn.pop_front();

            OnMessage(msg.remote, msg.msg);

            nMessageCount++;
        }
    }

    virtual void OnClientValidated(std::shared_ptr<Connection<T>> client) {}

  protected:
    /**
     * @brief on client connect event
     *
     * @param client
     * @return true
     * @return false
     */
    virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client) { return false; }

    /**
     * @brief on client disconnect event
     *
     * @param client
     */
    virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client) {}

    /**
     * @brief on message event
     *
     * @param client
     * @param msg
     */
    virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T> &msg) {}

  protected:
    using r_type::net::NetServerInterface<T>::m_qMessagesIn;
    using r_type::net::NetServerInterface<T>::m_deqConnections;
    using r_type::net::NetServerInterface<T>::m_asioContext;
    using r_type::net::NetServerInterface<T>::m_threadContext;
    using r_type::net::NetServerInterface<T>::m_asioSocket;
    using r_type::net::NetServerInterface<T>::m_clientEndpoint;
    using r_type::net::NetServerInterface<T>::m_tempBuffer;
    using r_type::net::NetServerInterface<T>::nIDCounter;
};
} // namespace net
} // namespace r_type