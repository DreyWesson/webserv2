#include "../../inc/AllHeaders.hpp"

/// @todo
// check path modifiers
// location

RequestConfig::RequestConfig(HttpRequestParser &request, Listen &host_port, DB &db, Client &client) : request_(request), client_(client), host_port_(host_port), db_(db)
{
    (void)client_;
    (void)host_port_;
}

RequestConfig::~RequestConfig() {}

const VecStr& RequestConfig::filterDataByDirectives(const std::vector<KeyMapValue>& targetServ, std::string directive, std::string location = "")
{
    modifierType_ = NONE;
    std::string locationExtract;

    for (size_t i = 0; i < targetServ.size(); ++i)
    {
        const MapStr &keyMap = targetServ[i].first;
        const VecStr &valueVector = targetServ[i].second;

        MapStr::const_iterator directiveIt = keyMap.find("directives");
        if (directiveIt != keyMap.end() && directiveIt->second == directive)
        {
            MapStr::const_iterator locationIt = keyMap.find("location");
            if (locationIt != keyMap.end())
            {
                locationExtract = checkModifier(locationIt->second);
                if (locationExtract == location)
                    return valueVector;
            }
        }
        modifierType_ = NONE;
    }

    static VecStr emptyVector;
    return emptyVector;
}


const VecStr& RequestConfig::checkRootDB(std::string directive)
{
    GroupedDBMap::const_iterator it;
    for (it = db_.rootDB.begin(); it != db_.rootDB.end(); ++it)
    {
        const std::vector<ConfigDB::KeyMapValue> &values = it->second;
        for (size_t i = 0; i < values.size(); ++i)
        {
            const MapStr &keyMap = values[i].first;
            const VecStr &valueVector = values[i].second;
            std::string location = keyMap.find("location")->second;
            const VecStr& dirValue = filterDataByDirectives(values, directive, location);
            if (!dirValue.empty())
                return valueVector;
        }
    }

    static VecStr emptyVector;
    return emptyVector;
}

const VecStr& RequestConfig::cascadeFilter(std::string directive, std::string location = "")
{
    /// @note important to first pre-populate data in cascades:
    // 1. preffered settings
    // 2. http level
    // 3. server level(locatn == "" == server-default settings)
    // 4. location level

    const VecStr& dirValue = filterDataByDirectives(targetServer_,directive, location);
    if (!dirValue.empty())
        return dirValue;

    if (dirValue.empty() && !location.empty()) {
        const VecStr& filteredValue = filterDataByDirectives(targetServer_, directive, "");
        if (!filteredValue.empty())
            return filteredValue;
        
    }

    return checkRootDB(directive);
}

std::string RequestConfig::checkModifier(const std::string &locationStr)
{
    std::string modifiers;
    std::string newStr;
    size_t j;
    for (j = 0; j < locationStr.size(); ++j)
    {
        char ch = locationStr[j];

        if (ch == '^' || ch == '*' || ch == '~' || ch == '=' || ch == '_')
        {
            if (ch != '_')
                modifiers += ch;
        }
        else
            break;
    }

    bool hasTilde = (modifiers.find("~") != std::string::npos);
    bool hasAsterisk = (modifiers.find("*") != std::string::npos);
    bool hasCaret = (modifiers.find("^") != std::string::npos);
    bool hasEquals = (modifiers.find("=") != std::string::npos);

    newStr = locationStr.substr(j);

    if (hasCaret && hasTilde)
        modifierType_ = LONGEST;
    else if (hasTilde && hasAsterisk)
        modifierType_ = CASE_INSENSITIVE;
    else if (hasEquals)
        modifierType_ = EXACT;
    else if (hasTilde)
        modifierType_ = CASE_SENSITIVE;
    return newStr;
}

/**
 * SETTERS
*/
void RequestConfig::setUp(size_t targetServerIdx)
{
    targetServer_ = getDataByIdx(db_.serversDB, targetServerIdx);

    setTarget(request_.getURI());
    setUri(request_.getURI());
    setRoot(cascadeFilter("root", target_));
    setClientMaxBodySize(cascadeFilter( "client_max_body_size", target_));
    setAutoIndex(cascadeFilter("autoindex", target_));

    /// @note debugging purpose
    getTarget();
    getUri();
    getRoot();
    getClientMaxBodySize();
    getAutoIndex();
    // printAllDBData(db_.serversDB);
    // printData(targetServer);
    // VecStr result = filterDataByDirectives(targetServer_, "autoindex", target_);
    VecStr result = cascadeFilter("default_type", target_);
    for (size_t i = 0; i < result.size(); ++i)
    {
        std::cout << "Value: " << result[i] << std::endl;
    }
}

void RequestConfig::setTarget(const std::string &target)
{
    target_ = target;
}

void RequestConfig::setRoot(const VecStr root)
{
    if (root.empty()) {
        root_ = "html";
        return;
    }
    root_ = root[0];
}

void RequestConfig::setUri(const std::string uri)
{
    uri_ = uri;
}

void RequestConfig::setClientMaxBodySize(const VecStr size)
{
    size_t val;
    if (size.empty()) {
        client_max_body_size_ = 20971520; //default 20mb
        return;
    }
    std::istringstream iss(size[0]);
    iss >> val;
    client_max_body_size_ = val;
}

void RequestConfig::setAutoIndex(const VecStr autoindex) {
    if (autoindex.empty()) {
        autoindex_ = false;
        return;
    }
    autoindex_ = (autoindex[0] == "on")? true : false;
}

// void RequestConfig::setIndexes(const VecStr indexes) {
//     if (indexes.empty()) {
//         indexes_ = &indexes;
//         return;
//     }
//     indexes_ = &indexes;
// }


/**
 * GETTERS
*/

std::string &RequestConfig::getTarget()
{
    std::cout << "target: " << target_ << "\n";
    return target_;
}

std::string &RequestConfig::getRequestTarget()
{
    return request_.getURI();
}

std::string &RequestConfig::getQuery()
{
    std::cout << "query: " <<  request_.getQuery() << "\n";
    return request_.getQuery();
}

std::string &RequestConfig::getFragment()
{
    std::cout << "fragment: " <<  request_.getFragment() << "\n";
    return request_.getFragment();
}

std::string &RequestConfig::getHost()
{
    std::cout << "ip: " <<  host_port_.ip_ << "\n";
    return host_port_.ip_;
}

uint32_t &RequestConfig::getPort()
{
    std::cout << "port: " <<  host_port_.port_ << "\n";
    return host_port_.port_;
}

Client &RequestConfig::getClient()
{
    return client_;
}



std::string &RequestConfig::getRoot()
{
    std::cout << "root: " << root_ << "\n";
    return root_;
}



std::string &RequestConfig::getUri()
{
    std::cout << "uri: " << uri_ << "\n";
    return uri_;
}



size_t &RequestConfig::getClientMaxBodySize()
{

    std::cout << "client_max_body_size: " << client_max_body_size_ << "\n";
    return client_max_body_size_;
}

bool RequestConfig::getAutoIndex() {

    std::cout << "autoindex: " << autoindex_ << "\n";
  return autoindex_;
}

// std::vector<std::string> &RequestConfig::getIndexes() {
//   return indexes_;
// }
