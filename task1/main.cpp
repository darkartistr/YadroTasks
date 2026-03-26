#include <iostream>
#include <cstdint>
#include <algorithm>

using namespace std;

class bitset {

private:
    uint8_t* bits;
    size_t length;

    void resize(size_t k) {
        size_t count = (length > 0) ? length : 8;
        while (count <= k) {
            count *= 2;
        }
        size_t newLengthBlock = (count + 7) / 8;
        size_t oldLengthBlock = (length + 7) / 8;
        uint8_t* newArray = new uint8_t[newLengthBlock]();
        if (bits != nullptr) {
            copy(bits, bits + oldLengthBlock, newArray);
            delete[] bits;
        }

        bits = newArray;
        length = count;
    }

public:
    bitset() {
        bits = nullptr;
        length = 0;
    }

    explicit bitset(size_t initial_capacity) {
        size_t newLengthBlock = (initial_capacity + 7) / 8;
        if (newLengthBlock > 0) {
            bits = new uint8_t[newLengthBlock]();
            length = initial_capacity;
        }
        else {
            bits = nullptr;
            length = 0;
        }
    }

    void set(size_t k, bool b) {
        if (k >= length) {
            resize(k);
        }
        size_t index = k / 8;
        size_t offset = k % 8;

        if (b) {
            bits[index] |= 1 << offset;
        }
        else {
            bits[index] &= ~(1 << offset);
        }
    }

    bool test(size_t k) const {
        if (k >= length) {
            return false;
        }
        size_t index = k / 8;
        size_t offset = k % 8;

        uint8_t currentBlock = bits[index];
        int bit = (currentBlock >> offset) & 1;
        return bit == 1 ? true : false;
    }

    bool operator[](size_t k) const {
        if (k >= length) {
            return false;
        }

        size_t index = k / 8;
        size_t offset = k % 8;

        uint8_t currentBlock = bits[index];
        int bit = (currentBlock >> offset) & 1;
        return bit == 1 ? true : false;
    }

    bool empty() const {
        size_t num_blocks = (length + 7) / 8;
        for (size_t i = 0; i < num_blocks; ++i) {
            if (bits[i] != 0) {
                return false;
            }
        }
        return true;
    }

    void clear() {
        size_t countLength = (length + 7) / 8;
        for (size_t k = 0; k < countLength; k++) {
            bits[k] = 0;
        }
    }

    bitset (const bitset& bit) {
        size_t newLenght = bit.length;
        uint8_t* newArray = new uint8_t[(newLenght + 7) / 8];
        copy(bit.bits, bit.bits + (newLenght + 7) / 8, newArray);
        this->bits = newArray;
        this->length = newLenght;
    }

    bitset (bitset&& other) noexcept {
        this->bits = other.bits;
        this->length = other.length;

        other.bits = nullptr;
        other.length = 0;
    }

    bitset& operator=(const bitset& other) {
        if (this == &other) return *this;
        size_t newLength = (other.length + 7) / 8;
        uint8_t* newArray = nullptr;

        if (newLength > 0) {
            newArray = new uint8_t[newLength]();
            if (other.bits) {
                std::copy(other.bits, other.bits + newLength, newArray);
            }
        }

        delete[] bits;
        this->length = other.length;
        this->bits = newArray;

        return *this;
    }

    bitset& operator=(bitset&& other) noexcept {
        if (this == &other) return *this;
        delete[] bits;
        this->bits = other.bits;
        this->length = other.length;

        other.bits = nullptr;
        other.length = 0;
        return *this;
    }

    bitset union_with(const bitset& other) const {
        size_t needLength = (max(this->length, other.length) + 7) / 8;
        bitset result(max(this->length, other.length));

        size_t thisBytes = (this->length + 7) / 8;
        size_t otherBytes = (other.length + 7) / 8;


        size_t first = min(thisBytes, otherBytes);

        size_t count = 0;
        for (; count < first; ++count) {
            result.bits[count] = other.bits[count] | this->bits[count];
        }

        if (this->length > other.length && this->bits) {
            for (; count < needLength; ++count) {
                result.bits[count] = this->bits[count];
            }
        }
        else if (other.bits) {
            for (; count < needLength; ++count) {
                result.bits[count] = other.bits[count];
            }
        }


        return result;
    }

    size_t size() const {
        return this->length;
    }

    bitset intersection(const bitset& other) const {

        size_t needLength = (min(this->length, other.length) + 7) / 8;
        bitset result(min(this->length, other.length));

        if (this->bits && other.bits) {
            for (size_t k = 0; k < needLength; ++k) {
                result.bits[k] = this->bits[k] & other.bits[k];
            }
        }

        return result;
    }
    bool is_subset(const bitset& other) const {
        size_t thisBytes = (this->length + 7) / 8;
        size_t otherBytes = (other.length + 7) / 8;

        for (size_t k = 0; k < thisBytes; ++k) {
            uint8_t otherByte = (k < otherBytes && other.bits) ? other.bits[k] : 0;
            uint8_t thisByte = (this->bits) ? this->bits[k] : 0;

            if ((thisByte & otherByte) != thisByte) {
                return false;
            }
        }

        return true;
    }

    ~bitset() {
        delete[] bits;
    }

};

int main() {
    bitset a(10);
    bitset b(20);

    a.set(5, true);
    a.set(65, true);

    b.set(5, true);
    b.set(15, true);

    bitset c = a.intersection(b);
    cout << "Intersection bit 5: " << c.test(5) << " (expected 1)" << endl;
    cout << "Intersection bit 15: " << c.test(15) << " (expected 0)" << endl;

    bitset sub(5);
    sub.set(2, true);
    bitset parent(10);
    parent.set(2, true);
    parent.set(7, true);

    cout << "Is subset: " << sub.is_subset(parent) << " (expected 1)" << endl;

    bitset d = parent;
    cout << "Copy test bit 7: " << d.test(7) << " (expected 1)" << endl;

    return 0;
}