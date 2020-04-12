/**
 * @file examples/info/access.cpp
 * @author Marcel Breyer
 * @date 2020-02-16
 *
 * @brief Code snippets for the @ref mpicxx::info access operator implementations.
 */

//! [access at]
mpicxx::info obj = { {"key", "foo"} };
try {
    obj.at("key") = "bar";                   // write access
    std::string str_val = obj.at("key");     // read access: returns a proxy object, which is immediately casted to a std::string
    str_val = "baz";                         // changing str_val will (obviously) not change the value of obj.at("key")

    // same as: obj.at("key") = "baz";
    auto val = obj.at("key");                // read access: returns a proxy object
    val = "baz";                             // write access: now obj.at("key") will return "baz"

    obj.at("key_2") = "baz";                 // will throw
} catch (const std::out_of_range& e) {
    std::cerr << e.what() << std::endl;      // prints: "key_2 doesn't exist!"
}
//! [access at]

//! [access const at]
const mpicxx::info obj = { {"key", "foo"} };
try {
    obj.at("key") = "bar";                      // write access: modifying a temporary is nonsensical
    std::string str_val = obj.at("key");        // read access: directly returns a std::string
    str_val = "baz";                            // changing str_val will (obviously) not change the value of obj.at("key")

    auto val = obj.at("key");                   // read access: directly returns a std::string
    val = "baz";                                // typeof val is std::string -> changing val will not change the value of obj.at("key)"

    std::string throw_val = obj.at("key_2");    // will throw
} catch (const std::out_of_range& e) {
    std::cerr << e.what() << std::endl;         // prints: "key_2 doesn't exist!"
}
//! [access const at]

//! [access operator overload]
mpicxx::info obj = { {"key", "foo"} };

obj["key"] = "bar";                 // write access
std::string str_val = obj["key"];   // read access: returns a proxy object, which is immediately casted to a std::string
str_val = "baz";                    // changing val won't alter obj["key"] !!!

// same as: obj["key"] = "baz";
auto val = obj["key"];              // read access: returns a proxy object
val = "baz";                        // write access: now obj["key"] will return "baz"

obj["key_2"] = "baz";               // inserts a new [key, value]-pair in obj
//! [access operator overload]