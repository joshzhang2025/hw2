#ifndef MYDATASTORE_H
#define MYDATASTORE_H

#include "datastore.h"
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>

class MyDataStore : public DataStore {
public:
    ~MyDataStore() override;

    void addProduct(Product* p) override;
    void addUser(User* u) override;
    std::vector<Product*> search(std::vector<std::string>& terms,
                                 int type) override;
    void dump(std::ostream& ofile) override;

    bool hasUser(const std::string& username) const;
    bool addToCart(const std::string& username, Product* product);
    bool viewCart(const std::string& username, std::ostream& os) const;
    bool buyCart(const std::string& username);

private:
    std::vector<Product*> products_;
    std::vector<User*> users_;

    std::map<std::string, std::set<Product*> > keywordIndex_;
    std::map<Product*, std::size_t> productPositions_;
    std::map<std::string, User*> usersByName_;
    std::map<std::string, std::vector<Product*> > carts_;

    std::vector<Product*> lastHits_;
};

#endif