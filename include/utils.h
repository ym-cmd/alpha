#include "Configuration.h"


#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>


class utils {
public:
    static utils* getInstance();

    std::unordered_map<string, int> filterStopWords(const std::vector<string>& vec); 
    double TF_IDF(int TF, int DF, size_t N);
    std::string extractTagContent(const std::string& doc, const std::string& tag);
    std::size_t length(const std::string& str);
    std::size_t nBytesCode(const char ch);
    int editDistance(const std::string& lhs, const std::string& rhs);
    bool isChinese(const std::string& word);


private:
    utils();
    void loadCnStopWords();
    void loadEnStopWords();

    static void destroy();
    static void init_r();

    utils(const utils& rhs) = delete;
    utils& operator=(const utils& rhs) = delete;
private:
    static utils* _pInstance;
    static pthread_once_t _once;

    std::unordered_set<std::string> _cnStopWordList;
    std::unordered_set<std::string> _enStopWordList;
};