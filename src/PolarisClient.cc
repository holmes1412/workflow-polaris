#include "src/PolarisClient.h"

namespace polaris {

PolarisClient::PolarisClient() {
    this->cluster = NULL;
    this->protocol = P_HTTP;
}

PolarisClient::~PolarisClient() { delete this->cluster; }

int PolarisClient::init(const std::string &url) {
    std::string protocol;
    if (strncasecmp(url.c_str(), "http://", 7) == 0) {
        this->protocol = P_HTTP;
        protocol = "http://";
    } else if (strncasecmp(url.c_str(), "trpc://", 7) == 0) {
        this->protocol = P_TRPC;
        protocol = "trpc://";
    } else {
        return -1;
    }
    this->cluster = new PolarisCluster;
    std::string::size_type pos = url.find(',');
    std::string::size_type ppos = 0;
    if (pos == std::string::npos) {
        std::string host = url;
        if (host.find("http://") != 0 && host.find("trpc://") != 0) {
            host = protocol + host;
        }
        this->cluster->get_server_connectors()->emplace_back(host);
    } else {
        do {
            std::string host = url.substr(ppos, pos - ppos);
            if (host.find("http://") != 0 || host.find("trpc://") != 0) {
                host = protocol + host;
            }
            this->cluster->get_server_connectors()->emplace_back(host);
            ppos = pos + 1;
            pos = host.find(',', ppos);
        } while (pos != std::string::npos);
    }

    if (this->cluster->get_server_connectors()->empty()) {
        return -1;
    }
    return 0;
}

PolarisTask *PolarisClient::create_discover_task(const std::string &service_namespace,
                                                 const std::string &service_name, int retry,
                                                 polaris_callback_t cb) {
    PolarisTask *task =
        new PolarisTask(service_namespace, service_name, retry, cluster, std::move(cb));

    task->set_apitype(DISCOVER);
    task->set_protocol(this->protocol);
    return task;
}

PolarisTask *PolarisClient::create_register_task(const std::string &service_namespace,
                                                 const std::string &service_name, int retry,
                                                 polaris_callback_t cb) {
    PolarisTask *task =
        new PolarisTask(service_namespace, service_name, retry, cluster, std::move(cb));
    task->set_apitype(REGISTER);
    task->set_protocol(this->protocol);
    return task;
}

PolarisTask *PolarisClient::create_deregister_task(const std::string &service_namespace,
                                                   const std::string &service_name, int retry,
                                                   polaris_callback_t cb) {
    PolarisTask *task =
        new PolarisTask(service_namespace, service_name, retry, cluster, std::move(cb));
    task->set_apitype(DEREGISTER);
    task->set_protocol(this->protocol);
    return task;
}

void PolarisClient::deinit() {
    delete this->cluster;
    this->cluster = NULL;
}

};  // namespace polaris

