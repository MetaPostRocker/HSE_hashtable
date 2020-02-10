#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    std::vector<std::list<std::pair<const KeyType, ValueType>>> table;
    size_t elemNum;
    size_t tableSize;
    Hash hasher;

    void initTable() {
        table.clear();
        table.resize(1);
        elemNum = 0;
        tableSize = 1;
    }

    size_t pos(const KeyType& key) const {
        return hasher(key) % tableSize;
    }

    void rebuild() {
        std::vector<std::pair<const KeyType, ValueType>> copy;
        for (auto& list : table) {
            for (auto& elem : list) {
                copy.emplace_back(elem);
            }
        }
        table.clear();
        elemNum = 0;
        table.resize(tableSize);
        for (auto& x : copy) {
            add(x);
        }
    }

    void add(const std::pair<KeyType, ValueType>& elem) {
        table[pos(elem.first)].emplace_back(elem);
        if (static_cast<double>(++elemNum) / static_cast<double>(tableSize) > 0.75) {
            tableSize *= 2;
            rebuild();
        }
    }

    void del(const KeyType& key) {
        size_t x = pos(key);
        for (auto it = table[x].begin(); it != table[x].end(); ++it) {
            if (it->first == key) {
                table[x].erase(it);
                elemNum--;
                break;
            }
        }
        if (static_cast<double>(elemNum) / static_cast<double>(tableSize) < 0.25) {
            tableSize /= 2;
            rebuild();
        }
    }

public:
    HashMap(const Hash& h = Hash()): hasher(h) {
        initTable();
    }

    template<class ItType>
    HashMap(ItType begin, ItType end, const Hash& h = Hash()): hasher(h) {
        initTable();
        while (begin != end) {
            add(*begin++);
        }
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>>& list, const Hash& h = Hash()) : hasher(h) {
        initTable();
        for (auto& elem : list) {
            add(elem);
        }
    }

    size_t size() const {
        return elemNum;
    }

    bool empty() const {
        return (elemNum == 0);
    }

    Hash hash_function() const {
        return hasher;
    }

    void clear() {
        initTable();
    }

    HashMap& operator = (const HashMap other) {
        clear();
        for (auto& elem : other) {
            add(elem);
        }
        return *this;
    }

    class iterator: public std::iterator<std::forward_iterator_tag, std::pair<const KeyType, ValueType>> {
    private:
        std::vector<std::list<std::pair<const KeyType, ValueType>>>* vec;
        int vecPos;
        typename std::list<std::pair<const KeyType, ValueType>>::iterator elemPointer;

    public:
        iterator() {}

        iterator(std::vector<std::list<std::pair<const KeyType, ValueType>>>* vecRef, int pos,
            typename std::list<std::pair<const KeyType, ValueType>>::iterator ptr):
            vec(vecRef), vecPos(pos) ,elemPointer(ptr) {}

        iterator& operator=(const iterator& other) {
            vec = other.vec;
            vecPos = other.vecPos;
            elemPointer = other.elemPointer;
            return *this;
        }

        iterator operator++() {
            elemPointer++;
            while (vecPos < (*vec).size() && elemPointer == (*vec)[vecPos].end()) {
                vecPos++;
                elemPointer = (*vec)[vecPos].begin();
            }
            if (vecPos == (*vec).size()) {
                vecPos--;
                elemPointer = (*vec)[vecPos].end();
            }
            return *this;
        }

        iterator operator++(int) {
            iterator duck = iterator(vec, vecPos, elemPointer);
            ++(*this);
            return duck;
        }

        std::pair<const KeyType, ValueType>& operator*() {
            return *elemPointer;
        }

        std::pair<const KeyType, ValueType>* operator->() {
            return &(*elemPointer);
        }

        bool operator==(const iterator& other) const {
            return elemPointer == other.elemPointer;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        bool operator==(const typename HashMap<KeyType, ValueType>::const_iterator& other) const {
            return elemPointer == other.elemPointer;
        }

        bool operator!=(const typename HashMap<KeyType, ValueType>::const_iterator& other) const {
            return !(*this == other);
        }
    };

    class const_iterator: public std::iterator<std::forward_iterator_tag, const std::pair<const KeyType, ValueType>> {
    private:
        const std::vector<std::list<std::pair<const KeyType, ValueType>>>* vec;
        int vecPos;
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator elemPointer;

    public:
        const_iterator() {}

        const_iterator(const std::vector<std::list<std::pair<const KeyType, ValueType>>>* vecRef, int pos,
            typename std::list<std::pair<const KeyType, ValueType>>::const_iterator ptr):
            vec(vecRef), vecPos(pos) ,elemPointer(ptr) {}

        const_iterator operator++() {
            elemPointer++;
            while (vecPos < (*vec).size() && elemPointer == (*vec)[vecPos].end()) {
                vecPos++;
                elemPointer = (*vec)[vecPos].begin();
            }
            if (vecPos == (*vec).size()) {
                vecPos--;
                elemPointer = (*vec)[vecPos].end();
            }
            return *this;
        }

        const_iterator& operator=(const const_iterator& other) {
            vec = other.vec;
            vecPos = other.vecPos;
            elemPointer = other.elemPointer;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator duck = const_iterator(vec, vecPos, elemPointer);
            ++(*this);
            return duck;
        }

        const std::pair<const KeyType, ValueType>& operator*() {
            return *elemPointer;
        }

        const std::pair<const KeyType, ValueType>* operator->() {
            return &(*elemPointer);
        }

        bool operator==(const iterator& other) const {
            return elemPointer == other.elemPointer;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        bool operator==(const const_iterator& other) const {
            return elemPointer == other.elemPointer;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }
    };

    iterator begin() {
        for (int i = 0; i < tableSize; ++i) {
            if (!table[i].empty()) {
                return iterator(&table, i, table[i].begin());
            }
        }
        return end();
    }

    const_iterator begin() const {
        for (int i = 0; i < tableSize; ++i) {
            if (!table[i].empty()) {
                return const_iterator(&table, i, table[i].begin());
            }
        }
        return end();
    }

    iterator end() {
        return iterator(&table, tableSize - 1, table[tableSize - 1].end());
    }

    const_iterator end() const {
        return const_iterator(&table, tableSize - 1, table[tableSize - 1].end());
    }

    iterator find(const KeyType& key) {
        size_t x = pos(key);
        for (auto it = table[x].begin(); it != table[x].end(); ++it) {
            if (it->first == key) {
                return iterator(&table, x, it);
            }
        }
        return end();
    }

    const_iterator find(const KeyType& key) const {
        size_t x = pos(key);
        for (auto it = table[x].begin(); it != table[x].end(); ++it) {
            if (it->first == key) {
                return const_iterator(&table, x, it);
            }
        }
        return end();
    }

    void insert(const std::pair<KeyType, ValueType>& elem) {
        if (find(elem.first) == end()) {
            add(elem);
        }
    }

    void erase(const KeyType& key) {
        if (find(key) != end()) {
            del(key);
        }
    }

    ValueType& operator[](const KeyType& key) {
        auto it = find(key);
        if (it == end()) {
            add({key, ValueType()});
            return find(key)->second;
        } else {
            return it->second;
        }
    }

    const ValueType& at(const KeyType& key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("out of range");
        } else {
            return it->second;
        }
    }
};
