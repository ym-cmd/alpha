#include <string>
#include <unordered_map>
#include <vector>
#include <set>

class WebPageQuery {
public:
    static WebPageQuery* getInstance();
    std::string doQuery(std::string keys);
private:
    void generateBaseVectors(std::vector<double>& base, const std::unordered_map<std::string, int>& afterCleanWords);
    std::string returnNoAnswer();
    std::string createJson(const std::vector<int>& topDocIDs, const std::vector<std::string>& docs);

private:
    static void destroy();
    static void init_r();

    WebPageQuery();
    void loadInvertIndex();
    void loadOffsetIndex();

    WebPageQuery(const WebPageQuery& rhs) = delete;
    WebPageQuery& operator=(const WebPageQuery& rhs) = delete;
private:
    static WebPageQuery* _pInstance;
    static pthread_once_t _once;

    std::unordered_map<std::string, std::set<std::pair<int, double>>> _invertIndex; // 词, docid, weight
    std::vector<std::pair<int, int>> _offsetIndex; // 下标docid pair (pos, length)
};