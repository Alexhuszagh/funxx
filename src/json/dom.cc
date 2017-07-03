//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see licenses/mit.md for more details.

#include <json/dom.h>
#include <stdexcept>

// OBJECTS
// -------


json_dom_handler::json_dom_handler(json_value_t& root):
    root(&root)
{}


void json_dom_handler::start_document()
{
    levels.emplace_back(root);
}


void json_dom_handler::end_document()
{
    levels.pop_back();
}


void json_dom_handler::start_object()
{
    add_value(new json_value_t(json_object_t()));
}


void json_dom_handler::end_object(size_t)
{
    levels.pop_back();
}


void json_dom_handler::start_array()
{
    add_value(new json_value_t(json_array_t()));
}


void json_dom_handler::end_array(size_t)
{
    levels.pop_back();
}


void json_dom_handler::key(const char* s, size_t n)
{
    has_key_ = true;
    key_ = json_string_t(s, n);
}


void json_dom_handler::null()
{
    add_value(new json_value_t(nullptr));
}


void json_dom_handler::boolean(bool v)
{
    add_value(new json_value_t(v));
}


void json_dom_handler::number(double d)
{
    add_value(new json_value_t(d));
}


void json_dom_handler::string(const char* s, size_t n)
{
    add_value(new json_value_t(json_string_t(s, n)));
}


void json_dom_handler::add_value(json_value_t* value)
{
    auto *parent = levels.back();
    if (levels.size() == 1 && parent->has_null()) {
        // root element
        std::swap(*parent, *value);
        delete value;
        return;
    } else if (has_key_) {
        parent->get_object().emplace(std::move(key_), value);
        has_key_ = false;
    } else {
        parent->get_array().emplace_back(value);
    }

    if (value->has_object() || value->has_array()) {
        levels.emplace_back(value);
    }
}


json_value_t::json_value_t():
    type_(json_null_type),
    data_((json_pointer_t) nullptr)
{}


json_value_t::json_value_t(json_value_t&& other):
    type_(json_null_type),
    data_((json_pointer_t) nullptr)
{
    swap(other);
}


json_value_t& json_value_t::operator=(json_value_t&& other)
{
    swap(other);
    return *this;
}


json_value_t::json_value_t(json_type type):
    type_(type),
    data_((json_pointer_t) nullptr)
{}


json_value_t::json_value_t(json_null_t):
    type_(json_null_type),
    data_((json_pointer_t) nullptr)
{}


json_value_t::json_value_t(bool value):
    type_(json_boolean_type),
    data_((json_pointer_t) value)
{}


json_value_t::json_value_t(double value):
    type_(json_number_type),
    data_((json_pointer_t) value)
{}


json_value_t::json_value_t(json_string_t&& value):
    type_(json_string_type),
    data_((json_pointer_t) new json_string_t(std::move(value)))
{}


json_value_t::json_value_t(json_array_t&& value):
    type_(json_array_type),
    data_((json_pointer_t) new json_array_t(std::move(value)))
{}


json_value_t::json_value_t(json_object_t&& value):
    type_(json_object_type),
    data_((json_pointer_t) new json_object_t(std::move(value)))
{}


json_value_t::~json_value_t()
{
    reset();
}


void json_value_t::swap(json_value_t& other)
{
    std::swap(type_, other.type_);
    std::swap(data_, other.data_);
}


json_type json_value_t::type() const
{
    return type_;
}


bool json_value_t::has_null() const
{
    return type() == json_null_type;
}


bool json_value_t::has_boolean() const
{
    return type() == json_boolean_type;
}


bool json_value_t::has_number() const
{
    return type() == json_number_type;
}


bool json_value_t::has_string() const
{
    return type() == json_string_type;
}


bool json_value_t::has_array() const
{
    return type() == json_array_type;
}


bool json_value_t::has_object() const
{
    return type() == json_object_type;
}


json_null_t json_value_t::get_null() const
{
    if (type_ != json_null_type) {
        throw std::runtime_error("Type is not null.");
    }
    return nullptr;
}


json_boolean_t json_value_t::get_boolean() const
{
    if (type_ != json_boolean_type) {
        throw std::runtime_error("Type is not boolean.");
    }
    return (json_boolean_t) data_;
}


json_number_t json_value_t::get_number() const
{
    if (type_ != json_number_type) {
        throw std::runtime_error("Type is not a number.");
    }
    return (json_number_t) data_;
}


json_string_t& json_value_t::get_string() const
{
    if (type_ != json_string_type) {
        throw std::runtime_error("Type is not a string.");
    }
    if (!data_) {
        throw std::runtime_error("Value is null.");
    }
    return * (json_string_t*) data_;
}


json_array_t& json_value_t::get_array() const
{
    if (type_ != json_array_type) {
        throw std::runtime_error("Type is not an array.");
    }
    if (!data_) {
        throw std::runtime_error("Value is null.");
    }
    return * (json_array_t*) data_;
}


json_object_t& json_value_t::get_object() const
{
    if (type_ != json_object_type) {
        throw std::runtime_error("Type is not an object.");
    }
    if (!data_) {
        throw std::runtime_error("Value is null.");
    }
    return * (json_object_t*) data_;
}


void json_value_t::set_null(json_null_t value)
{
    reset();
    data_ = (json_pointer_t) value;
    type_ = json_null_type;
}


void json_value_t::set_boolean(json_boolean_t value)
{
    reset();
    data_ = (json_pointer_t) value;
    type_ = json_boolean_type;
}


void json_value_t::set_number(json_number_t value)
{
    reset();
    data_ = (json_pointer_t) value;
    type_ = json_number_type;
}


void json_value_t::set_string(json_string_t&& value)
{
    reset();
    data_ = (json_pointer_t) new json_string_t(std::move(value));
    type_ = json_string_type;
}


void json_value_t::set_array(json_array_t&& value)
{
    reset();
    data_ = (json_pointer_t) new json_array_t(std::move(value));
    type_ = json_array_type;
}


void json_value_t::set_object(json_object_t&& value)
{
    reset();
    data_ = (json_pointer_t) new json_object_t(std::move(value));
    type_ = json_object_type;
}


void json_value_t::set(json_null_t value)
{
    set_null(value);
}


void json_value_t::set(json_boolean_t value)
{
    set_boolean(value);
}


void json_value_t::set(json_number_t value)
{
    set_number(value);
}


void json_value_t::set(json_string_t&& value)
{
    set_string(std::forward<json_string_t>(value));
}


void json_value_t::set(json_array_t&& value)
{
    set_array(std::forward<json_array_t>(value));
}


void json_value_t::set(json_object_t&& value)
{
    set_object(std::forward<json_object_t>(value));
}


void json_value_t::reset()
{
    switch (type_) {
        case json_null_type:
        case json_boolean_type:
        case json_number_type:
            break;
        case json_string_type:
            reset_string((json_string_t*) data_);
            break;
        case json_array_type:
            reset_array((json_array_t*) data_);
            break;
        case json_object_type:
            reset_object((json_object_t*) data_);
            break;
        default:
            throw std::runtime_error("Unexpected JSON value type.");
    }

    type_ = json_null_type;
}


void json_value_t::reset_string(json_string_t* value)
{
    delete value;
}


void json_value_t::reset_array(json_array_t* value)
{
    if (value) {
        for (json_value_t *v: *value) {
            delete v;
        }
    }
    delete value;
}


void json_value_t::reset_object(json_object_t* value)
{
    if (value) {
        for (auto& pair: *value) {
            delete pair.second;
        }
    }
    delete value;
}


void json_document_t::parse(std::istream& stream)
{
    json_stream_reader reader;
    json_dom_handler handler(*this);
    reader.set_handler(handler);
    reader.parse(stream);
}


void json_document_t::parse(const std::string& path)
{
    ifstream stream(path);
    parse(stream);
}


#if defined(HAVE_WFOPEN)

void json_document_t::parse(const std::wstring& path)
{
    ifstream stream(path);
    parse(stream);
}

#endif