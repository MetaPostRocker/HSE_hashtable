#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    static constexpr size_t resizeConst = 2;
    static constexpr double maxAlpha = 0.75;
    static constexpr double minAlpha = 0.25;

    std::vector<std::list<std::pair<const KeyType, ValueType>>> table;
    size_t elemNum;
    size_t bucketCount;
    Hash hasher;
    size_t minBucketId;

    void initTable() {
        table.clear();
        table.resize(1);
        elemNum = 0;
        minBucketId = 0;
        bucketCount = 1;
    }

    size_t getBucketId(const KeyType& key) const {
        return hasher(key) % bucketCount;
    }

    void rebuild() {
        std::vector<std::pair<const KeyType, ValueType>> copy;
        copy.reserve(table.size());
        for (const auto& list : table) {
            for (auto& elem : list) {
                copy.emplace_back(elem);
            }
        }
        table.clear();
        elemNum = 0;
        minBucketId = bucketCount - 1;
        table.resize(bucketCount);
        for (const auto& x : copy) {
            add(x);
        }
    }

    void add(const std::pair<KeyType, ValueType>& elem) {
        size_t temp = getBucketId(elem.first);
        if (temp < minBucketId) {
            minBucketId = temp;
        }
        table[temp].emplace_back(elem);
        if (static_cast<double>(++elemNum) / static_cast<double>(bucketCount) > maxAlpha) {
            bucketCount *= resizeConst;
            rebuild();
        }
    }

    void del(const KeyType& key) {
        size_t x = getBucketId(key);
        for (auto it = table[x].begin(); it != table[x].end(); ++it) {
            if (it->first == key) {
                table[x].erase(it);
                elemNum--;
                break;
            }
        }
        if (static_cast<double>(elemNum) / static_cast<double>(bucketCount) < minAlpha) {
            bucketCount /= resizeConst;
            rebuild();
        }
    }

    ValueType* findByHash(size_t h, const KeyType& key) {
        for (auto it = table[h].begin(); it != table[h].end(); ++it) {
            if (it->first == key) {
                return &(it->second);
            }
        }
        return nullptr;
    }

public:
    HashMap(const Hash& h = Hash()): hasher(h) {
        initTable();
    }

    HashMap(const HashMap& other): table(other.table), hasher(other.hasher) {
        elemNum = other.elemNum;
        bucketCount = other.bucketCount;
        minBucketId = other.minBucketId;
    }

    HashMap(HashMap&& other): table(std::move(other.table)), hasher(std::move(other.hasher)) {
        elemNum = other.elemNum;
        bucketCount = other.bucketCount;
        minBucketId = other.minBucketId;
    }

    template<class ItType>
    HashMap(ItType begin, ItType end, const Hash& h = Hash()): hasher(h) {
        initTable();
        while (begin != end) {
            add(*begin++);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, const Hash& h = Hash()) : hasher(h) {
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
        for (const auto& elem : other) {
            add(elem);
        }
        return *this;
    }

    HashMap& operator = (const HashMap&& other) {
        table = std::move(other.table);
        hasher = std::move(other.hasher);
        elemNum = other.elemNum;
        minBucketId = other.minBucketId;
        bucketCount = other.bucketCount;
        return *this;
    }

    class iterator: public std::iterator<std::forward_iterator_tag, std::pair<const KeyType, ValueType>> {
    private:
        std::vector<std::list<std::pair<const KeyType, ValueType>>>* vec;
        int elemBucketId;
        typename std::list<std::pair<const KeyType, ValueType>>::iterator elemPointer;

    public:
        iterator() {}

        iterator(std::vector<std::list<std::pair<const KeyType, ValueType>>>* vecRef, int pos,
            typename std::list<std::pair<const KeyType, ValueType>>::iterator ptr):
            vec(vecRef), elemBucketId(pos), elemPointer(ptr) {}

        iterator& operator=(const iterator& other) {
            vec = other.vec;
            elemBucketId = other.elemBucketId;
            elemPointer = other.elemPointer;
            return *this;
        }

        iterator operator++() {
            elemPointer++;
            while (elemBucketId < (*vec).size() && elemPointer == (*vec)[elemBucketId].end()) {
                elemBucketId++;
                elemPointer = (*vec)[elemBucketId].begin();
            }
            if (elemBucketId == (*vec).size()) {
                elemBucketId--;
                elemPointer = (*vec)[elemBucketId].end();
            }
            return *this;
        }

        iterator operator++(int) {
            iterator temp = iterator(vec, elemBucketId, elemPointer);
            ++(*this);
            return temp;
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
        int elemBucketId;
        typename std::list<std::pair<const KeyType, ValueType>>::const_iterator elemPointer;

    public:
        const_iterator() {}

        const_iterator(const std::vector<std::list<std::pair<const KeyType, ValueType>>>* vecRef, int pos,
            typename std::list<std::pair<const KeyType, ValueType>>::const_iterator ptr):
            vec(vecRef), elemBucketId(pos), elemPointer(ptr) {}

        const_iterator operator++() {
            elemPointer++;
            while (elemBucketId < (*vec).size() && elemPointer == (*vec)[elemBucketId].end()) {
                elemBucketId++;
                elemPointer = (*vec)[elemBucketId].begin();
            }
            if (elemBucketId == (*vec).size()) {
                elemBucketId--;
                elemPointer = (*vec)[elemBucketId].end();
            }
            return *this;
        }

        const_iterator& operator=(const const_iterator& other) {
            vec = other.vec;
            elemBucketId = other.elemBucketId;
            elemPointer = other.elemPointer;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = const_iterator(vec, elemBucketId, elemPointer);
            ++(*this);
            return temp;
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
        if (elemNum) return iterator(&table, minBucketId, table[minBucketId].begin());
        return end();
    }

    const_iterator begin() const {
        if (elemNum) return const_iterator(&table, minBucketId, table[minBucketId].begin());
        return end();
    }

    iterator end() {
        return iterator(&table, bucketCount - 1, table[bucketCount - 1].end());
    }

    const_iterator end() const {
        return const_iterator(&table, bucketCount - 1, table[bucketCount - 1].end());
    }

    iterator find(const KeyType& key) {
        size_t x = getBucketId(key);
        for (auto it = table[x].begin(); it != table[x].end(); ++it) {
            if (it->first == key) {
                return iterator(&table, x, it);
            }
        }
        return end();
    }

    const_iterator find(const KeyType& key) const {
        size_t x = getBucketId(key);
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
        del(key);
    }

    ValueType& operator[](const KeyType& key) {
        size_t hash = hasher(key);
        auto it = findByHash(hash % bucketCount, key);
        if (it == nullptr) {
            add({key, ValueType()});
            return *findByHash(hash % bucketCount, key);
        } else {
            return *it;
        }
    }

    const ValueType& at(const KeyType& key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("key not found");
        } else {
            return it->second;
        }
    }

    ~HashMap() {}
};
