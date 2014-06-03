// Copyright (c) Benjamin Kietzman (github.com/bkietz)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef DBUS_CONNECTION_SERVICE_HPP
#define DBUS_CONNECTION_SERVICE_HPP

#include <boost/asio.hpp>

#include <dbus/error.hpp>
#include <dbus/element.hpp>
#include <dbus/message.hpp>
#include <dbus/detail/async_send_op.hpp>

#include <dbus/impl/connection.ipp>

namespace dbus {

namespace bus {
  static const int session = DBUS_BUS_SESSION;
  static const int system  = DBUS_BUS_SYSTEM;
  static const int starter = DBUS_BUS_STARTER;
} // namespace bus

using namespace boost::asio;

class filter;
class match;

class connection_service
  : public io_service::service
{
public:
  static io_service::id id;

  typedef impl::connection implementation_type;

  explicit connection_service(io_service& io)
    : service(io)
  {
  }

  void construct(implementation_type& impl)
  {
  }

  void destroy(implementation_type& impl)
  {
  }

  void shutdown_service()
  {
    //TODO is there anything that needs shutting down?
  }

  void open(implementation_type& impl,
      const string& address, 
      bool shared)
  {
    io_service& io = this->get_io_service();

    impl = implementation_type(io, address, shared);
  }

  void open(implementation_type& impl,
      const int bus = bus::system,
      bool shared = true)
  {
    io_service& io = this->get_io_service();

    impl = implementation_type(io, bus, shared);
  }

  message send(implementation_type& impl,
      message& m)
  {
    error e;
    message reply(dbus_connection_send_with_reply_and_block(impl, 
        m, -1, e));

    e.throw_if_set();
    return reply;
  }

  template <typename Duration>
  message send(implementation_type& impl,
      message& m,
      const Duration& timeout)
  {
    //TODO generically convert timeout to milliseconds
    if(timeout == Duration::zero()) {
      //TODO this can return false if it failed
      dbus_connection_send(impl, m, NULL);
      return message();
    } else {
      error e;
      message reply(dbus_connection_send_with_reply_and_block(impl, 
          m, chrono::milliseconds(timeout).count(), e));

      e.throw_if_set();
      return reply;
    }
  }

  template<typename MessageHandler>
  inline BOOST_ASIO_INITFN_RESULT_TYPE(MessageHandler,
      void(boost::system::error_code, message))
  async_send(implementation_type& impl,
      message& m,
      BOOST_ASIO_MOVE_ARG(MessageHandler) handler)
  {
    // begin asynchronous operation
    impl.start(this->get_io_service());

    boost::asio::detail::async_result_init<
      MessageHandler, void(boost::system::error_code, message)> init(
        BOOST_ASIO_MOVE_CAST(MessageHandler)(handler));

    detail::async_send_op<
      BOOST_ASIO_HANDLER_TYPE(MessageHandler,
        void(boost::system::error_code, message))>(
          this->get_io_service(),
            BOOST_ASIO_MOVE_CAST(MessageHandler)(init.handler)) (impl, m);

    return init.result.get();
  }

  void new_match(implementation_type& impl,
      match& m);

  void delete_match(implementation_type& impl,
      match& m);
  
  void new_filter(implementation_type& impl,
      filter& f);
  
  void delete_filter(implementation_type& impl,
      filter& f);

};

io_service::id connection_service::id;




} // namespace dbus

#endif // DBUS_CONNECTION_SERVICE_HPP
