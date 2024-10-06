/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** netServer
*/

#pragma once

#include "netCommon.hpp"
#include "netConnection.hpp"
#include "netMessage.hpp"
#include "netThreadSafeQueue.hpp"

namespace r_type {
namespace net {
/**
 * @brief NetServerInterface class
 *
 * @tparam T
 */
template <typename T> class NetServerInterface {
  public:
    /**
     * @brief Construct a new Server Interface object
     *
     * @param port
     */

    NetServerInterface(uint16_t port)
        : m_asioSocket(m_asioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)){};

    /**
     * @brief Destroy the Server Interface object
     *
     */
    virtual ~NetServerInterface(){};
    /**
     * @brief Start the server
     *
     * @return true
     * @return false
     */
    virtual bool Start() = 0;

    /**
     * @brief Stop the server
     *
     */
    virtual void Stop() = 0;

    /**
     * @brief wait for client connection
     *
     */
    virtual void WaitForClientMessage() = 0;

    /**
     * @brief send message message to client
     *
     * @param client
     * @param msg
     */
    virtual void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T> &msg) = 0;

    /**
     * @brief message all clients
     *
     * @param msg
     * @param pIgnoreClient
     */
    virtual void MessageAllClients(
        const Message<T> &msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr) = 0;

    /**
     * @brief update server
     *
     * @param nMaxMessages
     * @param bWait
     */
    virtual void Update(size_t nMaxMessages = -1, bool bWait = false) = 0;

    virtual void OnClientValidated(std::shared_ptr<Connection<T>> client) = 0;

  protected:
    /**
     * @brief on client connect event
     *
     * @param client
     * @return true
     * @return false
     */
    virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client) = 0;

    /**
     * @brief on client disconnect event
     *
     * @param client
     */
    virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client) = 0;

    /**
     * @brief on message event
     *
     * @param client
     * @param msg
     */
    virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T> &msg) = 0;

  public:
    ThreadSafeQueue<OwnedMessage<T>> m_qMessagesIn;

    std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;

    asio::io_context m_asioContext;
    std::thread m_threadContext;

    asio::ip::udp::socket m_asioSocket;
    asio::ip::udp::endpoint m_clientEndpoint;
    // std::shared_ptr<r_type::net::Connection<T>> m_clientEndpoint;
    std::array<uint8_t, 1024> m_tempBuffer;

    uint32_t nIDCounter = 10000;
};
} // namespace net
} // namespace r_type