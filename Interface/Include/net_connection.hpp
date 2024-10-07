/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** netConnection
*/

#pragma once

#include "net_common.hpp"
#include "net_message.hpp"
#include "net_thread_safe_queue.hpp"
#include "net_r_type_message.hpp"

#define UNUSED __attribute__((unused))

namespace r_type {
namespace net {
/**
 * @brief ServerInterface class
 *
 * @tparam T
 */
template <typename T> class NetServerInterface;
/**
 * @brief Connection class
 *
 * @tparam T
 */
template <typename T> class Connection : public std::enable_shared_from_this<Connection<T>> {
  public:
    /**
     * @brief Connection owner
     *
     */
    enum class owner
    {
        server,
        client
    };

  public:
    /**
     * @brief Construct a new Connection object
     *
     * @param parent owner of the connection
     * @param asioContext asio context of the connection
     * @param socket socket of the connection
     * @param qIn
     */
    Connection(owner parent, asio::io_context &asioContext, asio::ip::udp::socket socket,
        ThreadSafeQueue<OwnedMessage<T>> &qIn)
        : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
    {
        m_nOwnerType = parent;

        if (m_nOwnerType == owner::server) {
            m_nHandshakeOut =
                uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
            m_nHandshakeCheck = scramble(m_nHandshakeOut);
        } else {
            m_nHandshakeIn = 0;
            m_nHandshakeOut = 0;
        }
    }

    /**
     * @brief Destroy the Connection object
     *
     */
    virtual ~Connection() {}

    /**
     * @brief get the ID of the connection
     *
     * @return uint32_t
     */
    uint32_t GetID() const { return id; }

  public:
    /**
     * @brief Connect to client
     *
     * @param server
     * @param uid
     */
    void ConnectToClient(r_type::net::NetServerInterface<T> *server, uint32_t uid = 0)
    {
        if (m_nOwnerType == owner::server) {
            if (m_socket.is_open()) {
                id = uid;
                WriteValidation();
                ReadValidation(server);
            } else {
                std::cout << "Error: Connection is not open\n";
            }
        }
    }

    /**
     * @brief Connect to server
     *
     * @param endpoints
     */
    void ConnectToServer(const asio::ip::udp::resolver::results_type &endpoints)
    {
        if (m_nOwnerType == owner::client) {
            asio::async_connect(m_socket, endpoints,
                [this](std::error_code ec, asio::ip::udp::endpoint UNUSED endpoint) {
                    if (!ec) {
                        ReadValidation();
                    }
                });
        }
    }

    /**
     * @brief Disconnect from server
     *
     */
    void Disconnect()
    {
        if (IsConnected())
            asio::post(m_asioContext, [this]() { m_socket.close(); });
    }

    /**
     * @brief return status of connection
     *
     * @return true
     * @return false
     */
    bool IsConnected() const { return m_socket.is_open(); }

    void StartListening() {}

  public:
    /**
     * @brief Send message to client
     *
     * @param msg
     */
    void Send(const Message<T> &msg)
    {
        asio::post(m_asioContext, [this, msg]() {
            bool bWritingMessage = !m_qMessagesOut.empty();
            m_qMessagesOut.push_back(msg);
            if (!bWritingMessage) {
                WriteHeader();
            }
        });
    }

  private:
    /**
     * @brief write header message to client or server
     *
     */
    void WriteHeader()
    {
        m_socket.async_send(asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader<T>)),
            [this](std::error_code ec, std::size_t UNUSED length) {
                if (!ec) {
                    if (m_qMessagesOut.front().body.size() > 0) {
                        WriteBody();
                    } else {
                        m_qMessagesOut.pop_front();

                        if (!m_qMessagesOut.empty()) {
                            WriteHeader();
                        }
                    }
                } else {
                    std::cout << "[" << id << "] Write Header Fail.\n";
                    m_socket.close();
                }
            });
    }

    /**
     * @brief write body message to client or server
     *
     */
    void WriteBody()
    {
        m_socket.async_send(
            asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
            [this](std::error_code ec, std::size_t UNUSED length) {
                if (!ec) {
                    m_qMessagesOut.pop_front();

                    if (!m_qMessagesOut.empty()) {
                        WriteHeader();
                    }
                } else {
                    std::cout << "[" << id << "] Write Body Fail.\n";
                    m_socket.close();
                }
            });
    }

    /**
     * @brief read header message from client or server
     *
     */
    void ReadHeader()
    {
        m_socket.async_receive(asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
            [this](std::error_code ec, std::size_t UNUSED length) {
                if (!ec) {
                    if (m_msgTemporaryIn.header.size > 0) {
                        m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                        ReadBody();
                    } else {
                        AddToIncomingMessageQueue();
                    }
                } else {
                    std::cout << "[" << id << "] Read Header Fail.\n";
                    m_socket.close();
                }
            });
    }

    /**
     * @brief read body message from client or server
     *
     */
    void ReadBody()
    {
        m_socket.async_receive(
            asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
            [this](std::error_code ec, std::size_t UNUSED length) {
                if (!ec) {
                    AddToIncomingMessageQueue();
                } else {
                    std::cout << "[" << id << "] Read Body Fail.\n";
                    m_socket.close();
                }
            });
    }

    /**
     * @brief add message to message queue
     *
     */
    void AddToIncomingMessageQueue()
    {
        if (m_nOwnerType == owner::server)
            m_qMessagesIn.push_back({this->shared_from_this(), m_msgTemporaryIn});
        else
            m_qMessagesIn.push_back({nullptr, m_msgTemporaryIn});

        ReadHeader();
    }

    /**
     * @brief scramble the input
     *
     * @param nInput
     * @return uint64_t
     */
    uint64_t scramble(uint64_t nInput)
    {
        uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
        out = (out & 0xF0F0F0F0DEADBEEF) << 4 | (out & 0xF0F0F0F00000000) >> 4;
        return out ^ 0xC0DEFACE12345678;
    }

    /**
     * @brief write validation to client
     *
     */
    void WriteValidation()
    {
        m_socket.async_send(asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
            [this](std::error_code ec, std::size_t UNUSED length) {
                if (!ec) {
                    if (m_nOwnerType == owner::client) {
                        ReadHeader();
                    }
                } else {
                    m_socket.close();
                }
            });
    }

    /**
     * @brief read validation from client
     *
     * @param server
     */
    void ReadValidation(r_type::net::NetServerInterface<T> *server = nullptr)
    {
        m_socket.async_receive(asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
            [this, server](std::error_code ec, std::size_t UNUSED length) {
                if (!ec) {
                    if (m_nOwnerType == owner::client) {
                        if (m_nHandshakeIn == m_nHandshakeCheck) {
                            std::cout << "Server Validated\n";
                            ReadHeader();
                        } else {
                            std::cout << "Server Disconnected (Fail Validation)\n";
                            m_socket.close();
                        }
                    } else {
                        m_nHandshakeOut = scramble(m_nHandshakeIn);
                        WriteValidation();
                    }
                } else {
                    m_socket.close();
                }
            });
    }

  protected:
    asio::ip::udp::socket m_socket;

    asio::io_context &m_asioContext;

    ThreadSafeQueue<Message<T>> m_qMessagesOut;

    ThreadSafeQueue<OwnedMessage<T>> &m_qMessagesIn;

    Message<T> m_msgTemporaryIn;

    owner m_nOwnerType = owner::server;

    uint32_t id = 0;

    uint64_t m_nHandshakeOut = 0;
    uint64_t m_nHandshakeIn = 0;
    uint64_t m_nHandshakeCheck = 0;
};
} // namespace net
} // namespace r_type