#ifndef _POLARISTASK_H_
#define _POLARISTASK_H_

#include "workflow/WFTaskFactory.h"
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "PolarisConfig.h"
#include "PolarisCluster.h"
#include <stdlib.h>
#include <functional>

namespace polaris {

enum PolarisProtocol {
    P_UNKNOWN,
    P_HTTP,
    P_TRPC,
};

enum ApiType {
    API_UNKNOWN = 0,
    API_DISCOVER,
    API_REGISTER,
    API_DEREGISTER,
    API_RATELIMIT,
    API_CIRCUITBREAKER,
    API_HEARTBEAT,
};

enum DiscoverRequestType {
    UNKNOWN = 0,
    INSTANCE,
    CLUSTER,
    ROUTING,
    RATELIMIT,
    CIRCUITBREAKER,
};

class PolarisTask;

using polaris_callback_t = std::function<void(PolarisTask *)>;
class PolarisTask : public WFGenericTask {
  public:
    PolarisTask(const std::string &snamespace, const std::string &sname,
                int retry_max, PolarisCluster *cluster, polaris_callback_t &&cb)
        : service_namespace(snamespace),
          service_name(sname),
          retry_max(retry_max),
          callback(std::move(cb)) {
        this->finish = false;
        this->apitype = API_UNKNOWN;
        this->protocol = P_UNKNOWN;
        int pos = rand() % cluster->get_server_connectors()->size();
        this->url = cluster->get_server_connectors()->at(pos);
        this->cluster = *cluster;
    }

    void set_apitype(ApiType apitype) { this->apitype = apitype; }

    void set_protocol(PolarisProtocol protocol) { this->protocol = protocol; }

    void set_config(const PolarisConfig &config) { this->config = config; }

    void set_service_token(const std::string &token) { this->service_token = token; }
    void set_platform_id(const std::string &id) { this->platform_id = id; }
    void set_platform_token(const std::string &token) { this->platform_token = token; }

    void set_polaris_instance(const PolarisInstance &instance) {
        this->polaris_instance = instance;
    }

    bool get_discover_result(struct discover_result *result) const;
    bool get_route_result(struct route_result *result) const;
    bool get_ratelimit_result(struct ratelimit_result *result) const;
    bool get_circuitbreaker_result(struct circuitbreaker_result *result) const;

  protected:
    virtual ~PolarisTask() {}
    WFHttpTask *create_discover_cluster_http_task();
    WFHttpTask *create_healthcheck_cluster_http_task();
    WFHttpTask *create_instances_http_task();
    WFHttpTask *create_route_http_task();
    WFHttpTask *create_register_http_task();
    WFHttpTask *create_deregister_http_task();
    WFHttpTask *create_ratelimit_http_task();
    WFHttpTask *create_circuitbreaker_http_task();
    WFHttpTask *create_heartbeat_http_task();

    static void discover_cluster_http_callback(WFHttpTask *task);
    static void healthcheck_cluster_http_callback(WFHttpTask *task);
    static void instances_http_callback(WFHttpTask *task);
    static void route_http_callback(WFHttpTask *task);
    static void register_http_callback(WFHttpTask *task);
    static void ratelimit_http_callback(WFHttpTask *task);
    static void circuitbreaker_http_callback(WFHttpTask *task);
    static void heartbeat_http_callback(WFHttpTask *task);

    std::string create_discover_request(const struct discover_request &request);
    std::string create_register_request(const struct register_request &request);
    std::string create_deregister_request(const struct deregister_request &request);
    std::string create_ratelimit_request(const struct ratelimit_request &request);
    std::string create_circuitbreaker_request(const struct circuitbreaker_request &request);

    bool parse_cluster_response(const std::string &body);
    int parse_instances_response(const std::string &body, std::string &revision);
    int parse_route_response(const std::string &body, std::string &revision);
    int parse_register_response(const std::string &body);
    int parse_ratelimit_response(const std::string &body, std::string &revision);
    int parse_circuitbreaker_response(const std::string &body, std::string &revision);

    virtual void dispatch();
    virtual SubTask *done();

    void check_failed(); // pull cluster instances again if failed many times

  private:
    std::string service_namespace;
    std::string service_name;
    int retry_max;
    polaris_callback_t callback;
    std::string url;
    std::string service_token;
    std::string platform_id;
    std::string platform_token;
    bool finish;
    ApiType apitype;
    PolarisProtocol protocol;
    std::string discover_res;
    std::string route_res;
    std::string ratelimit_res;
    std::string circuitbreaker_res;
    PolarisInstance polaris_instance;
    PolarisConfig config;
    PolarisCluster cluster;
};

};  // namespace polaris

#endif
