#include <cstdio>
#include <cstring>
#include <string>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/completion_callback.h>

#include <json/json.h>
#include <json/reader.h>
#include <json/writer.h>

#include <alve/gecmi/driver.hpp>
#include <alve/gecmi/confusion.hpp>

#include <boost/thread/thread.hpp>

namespace alve { namespace gecmi {

using boost::lexical_cast;
using std::string;

// Boilerplate {{{
/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurrence of the <embed> tag that has these
/// attributes:
/// <pre>
///     type="application/x-nacl"
///     nacl="hello_world.nmf"
/// </pre>
class GecmiInstance : public pp::Instance {
    boost::thread* worker;
    pp::Var msg;
    boost::condition_variable cond;
    boost::mutex mut;
    bool set_finish;
public:
    explicit GecmiInstance(PP_Instance instance) : pp::Instance(instance), worker(0), set_finish(false) {}
    virtual ~GecmiInstance() {
        set_finish = true;
        //boost::unique_lock<boost::mutex> lock(mut);

    //while(!data_ready)
    //{
        //cond.wait(lock);
    //}
    //process_data();
        cond.notify_one();
        worker->join();
    }
    virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);

    /// Called by the browser to handle the postMessage() call in Javascript.
    /// Detects which method is being called from the message contents, and
    /// calls the appropriate function.  Posts the result back to the browser
    /// asynchronously.
    /// @param[in] var_message The message posted by the browser.
    virtual void HandleMessage(const pp::Var& var_message);

    void work( );

    void driver_message_callback( std::string const& msg, boost::any const& );
}; // }}}

// std::string matrix_as_json( counter_matrix_t const& cm ) {{{
Json::Value matrix_as_json( counter_matrix_t const& cm )
{
    Json::Value root;
    //Json::Writer writer;
    size_t row_count = cm.size1();
    size_t col_count = cm.size2();

    for( size_t i=0; i < row_count ; i++  )
    {
        for( size_t j=0; j < col_count ; j++ )
        {
            // Order is (x,y), not (row,column)...
            std::string key="("+boost::lexical_cast<std::string>(i)+","+
                boost::lexical_cast<std::string>(j) + ")";
            root[key] = boost::lexical_cast<std::string>( cm(i,j) );
        }
    }
    root["m1_count"] = row_count;
    root["m2_count"] = col_count;

    return root;
} // }}}

// std::string partial_matrix_as_json( counter_matrix_t const& cm, size_t offset, size_t entry_count ) {{{
Json::Value partial_matrix_as_json( counter_matrix_t const& cm , size_t offset, size_t entry_count)
{
    Json::Value root;
    //Json::Writer writer;
    size_t row_count = cm.size1();
    size_t col_count = cm.size2();


    for ( size_t idx = offset; idx < offset + entry_count; idx++ )
    {
        size_t j = idx % col_count ;
        size_t i = idx / col_count ;
        // Order is (x,y), not (row,column)...
        std::string key="("+boost::lexical_cast<std::string>(i)+","+
            boost::lexical_cast<std::string>(j) + ")";
        root[key] = boost::lexical_cast<std::string>( cm(i,j) );
    }

    root["m1_count"] = row_count;
    root["m2_count"] = col_count;

    return root;
} // }}}

struct late_post_message_t{
    std::string msg;
    GecmiInstance* inst;
};

void late_post_message( void* data, int32_t result )
{
    late_post_message_t* lpm = (late_post_message_t*) data; 
    lpm->inst->PostMessage( lpm->msg );
    delete lpm;
}

// void ...::driver_message_callback( std::string const& msg, boost::any what ){{{
void GecmiInstance::driver_message_callback( std::string const& msg, boost::any const& what )
{
    std::string json_string;
    if ( msg == "info" )
    {
        Json::Value root;
        Json::StyledWriter writer;
        root["kind"] = "info";
        root["msg"] = boost::any_cast<std::string>(what);
        json_string = writer.write( root );
    } else if ( msg == "json_failed" )
    {
        Json::Value root;
        Json::StyledWriter writer;
        root["kind"] = "error";
        root["msg"] = "EarlyFailure: json message parsing failed";
        json_string = writer.write( root );
    } else if ( msg == "fail" )
    {
        Json::Value root;
        Json::StyledWriter writer;
        root["kind"] = "fail";
        json_string = writer.write( root );
    } else if ( msg=="error" )
    {
        Json::Value root;
        Json::StyledWriter writer;
        root["kind"] = "error";
        root["msg"] = boost::any_cast<std::string>(what);
        json_string = writer.write( root );
    } else if ( msg == "matrix" )
    {
        Json::Value root;
        Json::StyledWriter writer;
        root["kind"] = "matrix";
        root["matrix"] =
            matrix_as_json( boost::any_cast<counter_matrix_t>(what) );

        json_string = writer.write( root );
    } else if ( msg == "parmatrix" )
    {
        // Necessary for transmitting chunks of the matrix
        Json::Value root;
        Json::StyledWriter writer;
        root["kind"] = "parmatrix";
        // TODO: Your are copying the matrix!!
        counter_matrix_t cmat = boost::any_cast<counter_matrix_t>(what);
        size_t entry_count = cmat.size1() * cmat.size2();
        size_t CHUNK_SIZE=200;
        for ( size_t i=0; i < ( entry_count / CHUNK_SIZE + 1 ); i++ )
        {
            size_t offset = i * CHUNK_SIZE;
            size_t sz = std::min( CHUNK_SIZE, entry_count - offset );
            root["matrix"] =
                partial_matrix_as_json(cmat, offset, sz  );

            json_string = writer.write( root );
            late_post_message_t* lpm = new late_post_message_t();
            lpm->msg = json_string;
            lpm->inst = this;
            pp::Module::Get()->core()->CallOnMainThread(
                0,
                pp::CompletionCallback(&late_post_message, lpm )
            );
        }
        root.clear();
        root["kind"] = "debug";
        root["msg"] = "matrix_sent";
        json_string = writer.write( root );
    } else if ( msg == "debug" )
    {
        Json::Value root;
        Json::StyledWriter writer;
        root["kind"] = "debug";
        root["message"] =
            boost::any_cast<std::string>(what);

        json_string = writer.write( root );
    } else if ( msg == "result" )
    {
        Json::Value root;
        Json::StyledWriter writer;
        root["kind"] = "result";
        root["result"] =
            boost::any_cast<double>(what);

        json_string = writer.write( root );
    }
    late_post_message_t* lpm = new late_post_message_t();
    lpm->msg = json_string;
    lpm->inst = this;
    pp::Module::Get()->core()->CallOnMainThread(
        0,
        pp::CompletionCallback(&late_post_message, lpm )
    );
} // }}}

bool GecmiInstance::Init(uint32_t argc, const char* argn[], const char* argv[]) {
    worker = new boost::thread( boost::bind( &GecmiInstance::work, this ) );
    return true;
}

void GecmiInstance::work( ) {
    while( not set_finish )
    {
        boost::unique_lock<boost::mutex> lock(mut);
        while(!msg.is_string() )
        {
            cond.wait(lock);
            if ( set_finish )
                return ;
        }
        // From here down, all the code is protected by the lock
        //
        //
        pp::Var var_message = msg;
        msg = pp::Var(); // <-- EMPTY, to avoid infinite loop

        message_callback_t msg_callback = boost::bind( 
            &GecmiInstance::driver_message_callback, this, _1, _2 );
        // Expect to get the files
        std::string big_json = var_message.AsString();
        Json::Reader reader;
        Json::Value root;
        bool parse_ok = reader.parse( big_json, root );
        if ( not parse_ok )
        {
            msg_callback( "json_failed", 0 );
        }
        std::string file_1 = root.get("file_1", "").asString();
        std::string file_2 = root.get("file_2", "").asString();

        // And let things go
        double nmi = 0.0;
        bool failed = false;
        try {
            nmi = nmi_approximate_routine( file_1, file_2, 0.0001, msg_callback );
        } catch ( std::runtime_error const& e )
        {
            // Nothing to do
            failed = true;
            msg_callback( "fail", 0 );
        }
        if ( not failed ) 
        {
            msg_callback( "result", nmi );
        }
    }

  //PostMessage(return_var);
}

void GecmiInstance::HandleMessage(const pp::Var& var_message) {
	// REMEMBER: the runtime doesn't handle extremely long messages.
	// TODO: Pass the messages by bits
    driver_message_callback("debug", string("In HandleMessage"));

    // Tries to lock a resource so that the msg-variable doesn't get
    // messed up.
    boost::unique_lock< boost::mutex > lock( mut, boost::try_to_lock_t() );
    if ( lock.owns_lock() )
    {
        msg = var_message;
        cond.notify_one();
    } else
    {
        driver_message_callback("fail", 0);
    }
}

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with
/// <code>type="application/x-nacl"</code>.
class GecmiModule : public pp::Module {
 public:
  GecmiModule() : pp::Module() {}
  virtual ~GecmiModule() {}

  /// Create and return a GecmiInstance object.
  /// @param[in] instance a handle to a plug-in instance.
  /// @return a newly created GecmiInstance.
  /// @note The browser is responsible for calling @a delete when done.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new GecmiInstance(instance);
  }
};
}}   // namespace alve::gecmi


namespace pp {
/// Factory function called by the browser when the module is first loaded.
/// The browser keeps a singleton of this module.  It calls the
/// CreateInstance() method on the object you return to make instances.  There
/// is one instance per <embed> tag on the page.  This is the main binding
/// point for your NaCl module with the browser.
/// @return new GecmiModule.
/// @note The browser is responsible for deleting returned @a Module.
Module* CreateModule() {
  return new alve::gecmi::GecmiModule();
}
}  // namespace pp

