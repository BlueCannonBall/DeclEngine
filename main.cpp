#include "Polyweb/polyweb.hpp"
#include "json.hpp"
#include <boost/process.hpp>

namespace bp = boost::process;

int main() {
    pn::init();
    pn::UniqueSock<pw::Server> server;

    server->route("/word_info", 
        pw::HTTPRoute {
            [](const pw::Connection&, const pw::HTTPRequest& req, void*) {
                pw::QueryParameters::map_type::const_iterator word_it;
                if ((word_it = req.query_parameters->find("word")) == req.query_parameters->end()) {
                    return pw::HTTPResponse::make_basic(400);
                }

                
            },
        });

    if (server->bind("0.0.0.0", 8000) == PN_ERROR) {
        std::cerr << "Error: " << pn::universal_strerror() << std::endl;
        return 1;
    }

    if (server->listen() == PN_ERROR) {
        std::cerr << "Error: " << pw::universal_strerror() << std::endl;
        return 1;
    }

    pn::quit();
    return 0;
}
