#include "mydatastore.h"
#include "util.h"

MyDataStore::~MyDataStore()
{
    for (std::size_t i = 0; i < products_.size(); ++i) {
        delete products_[i];
    }

    for (std::size_t i = 0; i < users_.size(); ++i) {
        delete users_[i];
    }
}

void MyDataStore::addProduct(Product* p)
{
    if (p == NULL) {
        return;
    }

    productPositions_[p] = products_.size();
    products_.push_back(p);

    std::set<std::string> words = p->keywords();
    for (std::set<std::string>::iterator it = words.begin();
         it != words.end(); ++it) {
        keywordIndex_[convToLower(*it)].insert(p);
    }
}

void MyDataStore::addUser(User* u)
{
    if (u == NULL) {
        return;
    }

    users_.push_back(u);
    usersByName_[convToLower(u->getName())] = u;
}

std::vector<Product*> MyDataStore::search(
    std::vector<std::string>& terms, int type)
{
    lastHits_.clear();

    if (terms.empty() || (type != 0 && type != 1)) {
        return lastHits_;
    }

    std::set<Product*> matches;
    bool firstTerm = true;

    for (std::vector<std::string>::iterator term = terms.begin();
         term != terms.end(); ++term) {
        std::string word = convToLower(*term);
        std::map<std::string, std::set<Product*> >::iterator found =
            keywordIndex_.find(word);

        std::set<Product*> termMatches;
        if (found != keywordIndex_.end()) {
            termMatches = found->second;
        }

        if (firstTerm) {
            matches = termMatches;
            firstTerm = false;
        } else if (type == 0) {
            matches = setIntersection(matches, termMatches);
        } else {
            matches = setUnion(matches, termMatches);
        }
    }

    // Put hits in the same order that products were added.
    std::map<std::size_t, Product*> ordered;
    for (std::set<Product*>::iterator it = matches.begin();
         it != matches.end(); ++it) {
        ordered[productPositions_[*it]] = *it;
    }

    for (std::map<std::size_t, Product*>::iterator it = ordered.begin();
         it != ordered.end(); ++it) {
        lastHits_.push_back(it->second);
    }

    return lastHits_;
}

void MyDataStore::dump(std::ostream& ofile)
{
    ofile << "<products>\n";
    for (std::size_t i = 0; i < products_.size(); ++i) {
        products_[i]->dump(ofile);
    }
    ofile << "</products>\n";

    ofile << "<users>\n";
    for (std::size_t i = 0; i < users_.size(); ++i) {
        users_[i]->dump(ofile);
    }
    ofile << "</users>\n";
}

bool MyDataStore::hasUser(const std::string& username) const
{
    return usersByName_.find(convToLower(username)) != usersByName_.end();
}

bool MyDataStore::addToCart(const std::string& username, Product* product)
{
    std::string key = convToLower(username);

    if (!hasUser(username) ||
        product == NULL ||
        productPositions_.find(product) == productPositions_.end()) {
        return false;
    }

    carts_[key].push_back(product);
    return true;
}

bool MyDataStore::viewCart(const std::string& username,
                           std::ostream& os) const
{
    if (!hasUser(username)) {
        return false;
    }

    std::string key = convToLower(username);
    std::map<std::string, std::vector<Product*> >::const_iterator cart =
        carts_.find(key);

    if (cart == carts_.end()) {
        return true;
    }

    for (std::size_t i = 0; i < cart->second.size(); ++i) {
        os << "Item " << i + 1 << '\n'
           << cart->second[i]->displayString() << '\n';
    }

    return true;
}

bool MyDataStore::buyCart(const std::string& username)
{
    std::string key = convToLower(username);
    std::map<std::string, User*>::iterator user = usersByName_.find(key);

    if (user == usersByName_.end()) {
        return false;
    }

    std::map<std::string, std::vector<Product*> >::iterator cart =
        carts_.find(key);

    if (cart == carts_.end()) {
        return true;
    }

    std::vector<Product*> remaining;
    for (std::size_t i = 0; i < cart->second.size(); ++i) {
        Product* product = cart->second[i];

        if (product->getQty() > 0 &&
            user->second->getBalance() >= product->getPrice()) {
            product->subtractQty(1);
            user->second->deductAmount(product->getPrice());
        } else {
            remaining.push_back(product);
        }
    }

    cart->second.swap(remaining);
    return true;
}