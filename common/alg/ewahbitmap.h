/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_EWAHBITMAP_H
#define CXX_ALG_EWAHBITMAP_H
#include <algorithm>
#include <vector>
#include <string>
#include <cassert>
#include <iterator>
#include <stdint.h>
#include "common/config.h"

namespace cxx {
    namespace alg {

        namespace detail {

#ifdef  __GNUC__
            uint32_t    countOnes(uint32_t x)
            {
                return static_cast<uint32_t >(__builtin_popcount(x));
            }

            uint32_t    numberOfTrailingZeros(uint32_t x)
            {
                if(x == 0) return 32;
                return __builtin_ctz(x);
            }

            uint32_t    numberOfTrailingZeros(uint16_t x)
            {
                if(x == 0) return 16;
                return __builtin_ctz(x);
            }
#else
            uint32_t    countOnes(uint32_t x)
            {
                uint32_t c;
                for(c = 0; x; c++) {
                    x &= x - 1;
                }
                return c;
            }

            uint32_t numberOfTrailingZeros(uint32_t v)
            {
                if(v == 0) return 32;
                uint32_t c = 1;
                if((v & 0xffff) == 0) {
                    v >>= 16;
                    c += 16;
                }
                if((v & 0xff) == 0) {
                    v >>= 8;
                    c += 8;
                }
                if((v & 0xf) == 0) {
                    v >>= 4;
                    c += 4;
                }
                if((v & 0x3) == 0) {
                    v >>= 2;
                    c += 2;
                }
                c -= v & 0x1;
                return c;
            }

            uint32_t numberOfTrailingZeros(uint16_t v)
            {
                if(v == 0) return 16;
                uint32_t c = 1;
                if((v & 0xff) == 0) {
                    v >>= 8;
                    c += 8;
                }
                if((v & 0xf) == 0) {
                    v >>= 4;
                    c += 4;
                }
                if((v & 0x3) == 0) {
                    v >>= 2;
                    c += 2;
                }
                c -= v & 0x1;
                return c;
            }
#endif
            uint32_t countOnes(uint64_t x)
            {
                return countOnes(static_cast<uint32_t >(x)) +
                        countOnes(static_cast<uint32_t >(x >> 32));
            }

            uint32_t countOnes(uint16_t x)
            {
                return countOnes(static_cast<uint32_t >(x));
            }

            uint32_t numberOfTrailingZeros(uint64_t x)
            {
                if(static_cast<uint32_t >(x) != 0) {
                    return numberOfTrailingZeros(static_cast<uint32_t >(x));
                }
                else {
                    return 32 + numberOfTrailingZeros(static_cast<uint32_t >(x >> 32));
                }
            }

            template<typename uword >
            std::string to_string(uword x)
            {
                std::string str;
                str.reserve(sizeof(uword) * 8);
                for(uword k = 0; k < sizeof(uword) * 8; ++k) {
                    if(x & (static_cast<uword >(1) << k))
                        str += "1";
                    else
                        str += "0";
                }
                return str;
            }

            /** this class is used to represent a special type of word storing
             * a run length. it's defined by an Enhanced Word Aligned Hybrid
             * (EWAH) format. */
            template<typename uword >
            class RunLengthWord {
            public:
                static const uint32_t   RunningLengthBits               = sizeof(uword) * 4;
                static const uint32_t   LiteralBits                     = sizeof(uword) * 8 - 1 - RunningLengthBits;
                static const uword  LargestLiteralCount                 = (static_cast<uword >(1) << LiteralBits) - 1;
                static const uword  LargestRunningLengthCount           = (static_cast<uword >(1) << RunningLengthBits) - 1;
                static const uword  ShiftedLargestRunningLengthCount    = LargestRunningLengthCount << 1;
                static const uword  NotShiftedLargestRunningLengthCount = static_cast<uword >(~ShiftedLargestRunningLengthCount);
                static const uword  RunningLengthPlusRunningBit         = (static_cast<uword >(1) << (RunningLengthBits + 1)) - 1;
                static const uword  NotRunningLengthPlusRunningBit      = static_cast<uword >(~RunningLengthPlusRunningBit);
                static const uword  NotLargestRunningLengthCount        = static_cast<uword >(~LargestRunningLengthCount);

                RunLengthWord(uword & data) : data_(data) {}
                RunLengthWord(const RunLengthWord& rhs) : data_(rhs.data_) {}
                RunLengthWord& operator= (const RunLengthWord& rhs) {
                    if(this == &rhs) {
                        return *this;
                    }
                    data_ = rhs.data_;
                    return *this;
                }
                /** which bit is being repeated */
                bool get_running_bit() const {
                    return data_ & static_cast<uword >(1);
                }
                static bool get_running_bit(uword data) {
                    return data & static_cast<uword >(1);
                }
                /** how many words should be filled by the running bit */
                uword get_running_len() const {
                    return (data_ >> 1) & LargestRunningLengthCount;
                }
                static uword get_running_len(uword data) {
                    return (data >> 1) & LargestRunningLengthCount;
                }
                void set_running_bit(bool b) {
                    if(b) data_ |= static_cast<uword >(1);
                    else  data_ &= static_cast<uword >(~1);
                }
                static void set_running_bit(uword& data, bool b) {
                    if(b) data |= static_cast<uword >(1);
                    else  data &= static_cast<uword >(~1);
                }
                void set_running_len(uword l) {
                    data_ |= ShiftedLargestRunningLengthCount;
                    data_ &= static_cast<uword >(l << 1) | NotShiftedLargestRunningLengthCount;
                }
                static void set_running_len(uword& data, uword l) {
                    data |= ShiftedLargestRunningLengthCount;
                    data &= static_cast<uword >(l << 1) | NotShiftedLargestRunningLengthCount;
                }
                /** followed by how many literal words */
                uword get_literal_len() const {
                    return static_cast<uword >(data_ >> (1 + RunningLengthBits));
                }
                static uword get_literal_len(uword data) {
                    return static_cast<uword >(data >> (1 + RunningLengthBits));
                }
                void  set_literal_len(uword l) {
                    data_ |= NotRunningLengthPlusRunningBit;
                    data_ &= static_cast<uword >(l << (RunningLengthBits + 1)) | RunningLengthPlusRunningBit;
                }
                static void set_literal_len(uword& data, uword l) {
                    data |= NotRunningLengthPlusRunningBit;
                    data &= static_cast<uword >(l << (RunningLengthBits + 1)) | RunningLengthPlusRunningBit;
                }

                /** total of get_running_len() and get_literal_len() */
                uword size() const {
                    return get_running_len() + get_literal_len();
                }
                static uword size(uword data) {
                    return get_running_len(data) + get_literal_len(data);
                }

                const uword& data() const {
                    return data_;
                }
                void discard_first_words(uword x) {
                    assert(x <= size());
                    uword rl(get_running_len());
                    if(rl >= x) {
                        set_running_len(rl - x);
                        return;
                    }
                    x -= rl;
                    set_running_len(0);
                    set_literal_len(get_literal_len() - x);
                }
            private:
                uword&  data_;
            };

            template<typename uword = uint32_t >
            class ConstRunLengthWord {
            public:
                ConstRunLengthWord() : data_(0) {}
                ConstRunLengthWord(uword d) : data_(d) {}
                ConstRunLengthWord(const ConstRunLengthWord& rhs) : data_(rhs.data_) {}

                bool get_running_bit() const {
                    return data_ & static_cast<uword >(1);
                }
                uword get_running_len() const {
                    return (data_ >> 1) & RunLengthWord<uword >::LargestRunningLengthCount;
                }
                uword get_literal_len() const {
                    return static_cast<uword >(data_ >> (1 + RunLengthWord<uword >::RunningLengthBits));
                }
                uword size() const {
                    return get_running_len() + get_literal_len();
                }

                uword   data_;
            };

            /** same as RunLengthWord except value buffered for quick access */
            template<typename uword = uint32_t >
            class BufferedRunLengthWord {
            public:
                BufferedRunLengthWord(uword data) { read(data); }
                BufferedRunLengthWord(const RunLengthWord<uword >& p) { read(p.data()); }

                bool  get_running_bit() const { return running_bit_; }
                uword get_running_len() const { return running_len_; }
                uword get_literal_len() const { return literal_len_; }
                void  discard_first_words(uword x) {
                    assert(x <= size());
                    if(running_len_ >= x) {
                        running_len_ = static_cast<uword >(running_len_ - x);
                    }
                    else {
                        x = static_cast<uword >(x - running_len_);
                        running_len_ = 0;
                        literal_len_ = static_cast<uword >(literal_len_ - x);
                    }
                }
                uword size() const {
                    return running_len_ + literal_len_;
                }
                void  read(uword data) {
                    running_bit_ = (data & static_cast<uword >(1));
                    running_len_ = ((data >> 1) & RunLengthWord<uword >::LargestRunningLengthCount);
                    literal_len_ = (static_cast<uword >(data >> (1 + RunLengthWord<uword >::RunningLengthBits)));
                }
            private:
                bool    running_bit_;
                uword   running_len_;
                uword   literal_len_;
            };
        }

        ////////////////////////////////////////////////////////////////////////

        template<typename uword >
        class ewah_bitmap_iterator;
        template<typename uword >
        class ewah_bitmap_forward_iteartor;
        template<typename uword >
        class ewah_bitmap_raw_iterator;
        class bitmap_statistics;

        /** compressed bitmap using std::vector */
        template<typename uword = uint32_t >
        class ewah_bitmap {
        public:
            typedef ewah_bitmap_forward_iteartor<uword >    const_iterator;
            typedef std::vector<uword >                     container;
            typedef std::vector<size_t >                    bit_array;

            ewah_bitmap();

            /** set the bitmap size. padding with zeros if necessary */
            void            make_same_size(ewah_bitmap& r) {
                if(r.bitlen() < bitlen_) {
                    r.padding(bitlen_);
                }
                else if(bitlen_ < r.bitlen()) {
                    padding(r.bitlen());
                }
            }

            /** set the ith bit to true (starting from zero)
             * Note: you must set the bits in increasing order */
            void            set(size_t i);
            size_t          add(uword data, size_t bits = 8 * sizeof(uword));
            void            reset();
            /** return the number of bits set to the value 1.
             * the running time complexity is proportional to the compressed
             * size of the bitmap */
            size_t          count() const;

            void            debug() const;

            /** return an iterator that can be used to access the position of
             * set bits. the time complexity of a full scan is proportional to
             * the number of set bits. if you have very long strings of 1s, this
             * will be very inefficient.
             * it can be faster to use bitset() */
            const_iterator  begin() const {
                return ewah_bitmap_forward_iteartor<uword >(buffer_);
            }
            const_iterator  end() const {
                return ewah_bitmap_forward_iteartor<uword >(buffer_, buffer_.size());
            }
            /** iterate over the uncompressed words.
             * this is faster than begin() and end(). */
            ewah_bitmap_iterator<uword >    uncompress() const {
                return ewah_bitmap_iterator<uword >(buffer_);
            }
            /** iterate over the compressed words.
             * this is faster than any other iterator. */
            ewah_bitmap_raw_iterator<uword > raw_iterator() const {
                return ewah_bitmap_raw_iterator<uword >(*this);
            }

            /** return the set bits. can be much faster than iterating through
             * the set bits with iterator */
            bit_array       bitset() const;
            /** return the size in bits of the bitmap (uncompressed size) */
            size_t          bitlen() const  { return bitlen_; }
            /** set size in bits. this does not affect the compressed size */
            void            bitlen(size_t s){ bitlen_ = s; }
            /** return underlying buffer. this is different with bitset(), which
             * return the bits setted (not zero). */
            const container& buffer() const { return buffer_; }
            container&       buffer() { return buffer_; }
            /** return the size of buffer in bytes. this is the storage cost */
            size_t          memory() const { return buffer_.size() * sizeof(uword); }
            /** append the content of other compressed bitmap */
            void            append(const ewah_bitmap<uword >& rhs);

            void            _and(const ewah_bitmap<uword >& rhs, ewah_bitmap<uword >& result) const;
            void            _or (const ewah_bitmap<uword >& rhs, ewah_bitmap<uword >& result) const;
            void            _not(ewah_bitmap<uword >& result) const;
            void            _not();

            /** computer the logical and with another compressed bitmap.
             * the time complexity is proportional to the sum of the compressed
             * bitmap size */
            ewah_bitmap<uword > operator & (const ewah_bitmap<uword >& rhs) const {
                ewah_bitmap<uword > result;
                this->_and(rhs, result);
                return result;
            }
            /** computer the logical or with another compressed bitmap.
             * the time complexity is proportional to the sum of the compressed
             * bitmap size */
            ewah_bitmap<uword > operator | (const ewah_bitmap<uword >& rhs) const {
                ewah_bitmap<uword > result;
                this->_or(rhs, result);
                return result;
            }
            /** comuter the logical not */
            ewah_bitmap<uword > operator ~ () const {
                ewah_bitmap<uword > result;
                this->_not(result);
                return result;
            }
            ewah_bitmap<uword > operator + (const ewah_bitmap<uword >& rhs) const {
                ewah_bitmap<uword > result(*this);
                result.append(rhs);
                return result;
            }
            ewah_bitmap<uword >& operator+=(const ewah_bitmap<uword >& rhs) {
                this->append(rhs);
                return *this;
            }

            bool operator== (const ewah_bitmap<uword >& rhs) const {
                if(bitlen_ != rhs.bitlen_) return false;
                if(buffer_.size() != rhs.buffer_.size()) return false;
                for(size_t i = 0; i < buffer_.size(); ++i) {
                    if(buffer_[i] != rhs.buffer_[i]) return false;
                }
                return true;
            }
        private:
            enum { word_in_bits = sizeof(uword) * 8 };

//            /** copy operation is expensive, disabled */
//            ewah_bitmap(const ewah_bitmap& rhs);
//            /** assign operation is expensive, disabled */
//            ewah_bitmap& operator= (const ewah_bitmap& rhs);

            size_t          padding(size_t s);
            size_t          add_stream_of_empty_words(bool v, size_t number);
            size_t          add_stream_of_dirty_words(const uword* v, size_t number);
            void            fast_add_stream_of_empty_words(bool v, size_t number);
            size_t          add_literal_word(uword data);
            size_t          add_empty_word(bool v);


            container   buffer_;
            size_t      bitlen_;    //< length in bits
            size_t      last_;
        };

        template<typename uword >
        class ewah_bitmap_iterator {
        public:
            typedef typename ewah_bitmap<uword >::container container;

            static const uword zero     = 0;
            static const uword notzero  = static_cast<uword >(~0);

            ewah_bitmap_iterator(const ewah_bitmap_iterator& o)
                : pointer_(o.pointer_), buffers_(o.buffers_), compressed_(o.compressed_),
                  literal_(o.literal_), rl_(o.rl_), lw_(o.lw_), b_(o.b_) {}
            bool has_next() const { return pointer_ < buffers_.size(); }
            uword next() {
                uword result;
                if(compressed_ < rl_) {
                    ++compressed_;
                    if(b_)  result = notzero;
                    else    result = zero;
                }
                else {
                    assert(literal_ < lw_);
                    ++literal_;
                    ++pointer_;
                    assert(pointer_ < buffers_.size());
                    result = buffers_[pointer_];
                }
                if((compressed_ == rl_) && (literal_ == lw_)) {
                    ++pointer_;
                    if(pointer_ < buffers_.size())
                        read_new_running_length_word();
                }
                return result;
            }
        private:
            ewah_bitmap_iterator(const container& buf) :
                pointer_(0), buffers_(buf), compressed_(0), literal_(0),
                rl_(0), lw_(0), b_(0) {
                if(pointer_ < buffers_.size())
                    read_new_running_length_word();
            }

            void read_new_running_length_word() {
                literal_ = 0;
                compressed_ = 0;
                detail::ConstRunLengthWord<uword > rlw(buffers_[pointer_]);
                rl_ = rlw.get_running_len();
                lw_ = rlw.get_literal_len();
                b_  = rlw.get_running_bit();
                if((rl_ == 0) && (lw_ == 0)) {
                    if(pointer_ < buffers_.size() - 1) {
                        ++pointer_;
                        read_new_running_length_word();
                    }
                    else {
                        assert(pointer_ >= buffers_.size() - 1);
                        pointer_ = buffers_.size();
                        assert(!has_next());
                    }
                }
            }

            friend class ewah_bitmap<uword >;
            size_t              pointer_;
            const container&    buffers_;
            uword               compressed_;
            uword               literal_;
            uword               rl_, lw_;
            bool                b_;
        };

        template<typename uword >
        class ewah_bitmap_forward_iteartor {
        public:
            enum { word_in_bits = sizeof(uword) * 8 };
            typedef std::forward_iterator_tag   iterator_category;
            typedef size_t*                     pointer;
            typedef size_t&                     reference;
            typedef size_t                      value_type;
            typedef ptrdiff_t                   difference_type;
            typedef ewah_bitmap_forward_iteartor<uword >    type_of_iterator;
            typedef typename ewah_bitmap<uword >::container container;

            ewah_bitmap_forward_iteartor(const ewah_bitmap_forward_iteartor& o)
                : buffers_(o.buffers_), pointer_(o.pointer_),
                  pre_off_(o.pre_off_), cur_off_(o.cur_off_), running_(o.running_)
            {}

            size_t          operator* () const { return cur_off_ + pre_off_; }
            difference_type operator- (const type_of_iterator& o) {
                // TODO:
            }
            bool            operator< (const type_of_iterator& o) const {
                assert(&buffers_ == &o.buffers_);
                if(&buffers_ != &o.buffers_) return false;
                if(pointer_ == buffers_.size()) return false;
                if(o.pointer_ == o.buffers_.size()) return true;
                if(pre_off_ < o.pre_off_) return true;
                if(pre_off_ > o.pre_off_) return false;
                if(cur_off_ < o.cur_off_) return true;
                return false;
            }
            bool            operator<=(const type_of_iterator& o) const {
                return ((*this) < o) || ((*this) == o);
            }
            bool            operator==(const type_of_iterator& o) const {
                if(pointer_ == buffers_.size() && o.pointer_ == o.buffers_.size()) {
                    return true;
                }
                return &buffers_ == &o.buffers_ && pointer_ == o.pointer_ &&
                        pre_off_ == o.pre_off_ && cur_off_ == o.cur_off_;
            }
            bool            operator!=(const type_of_iterator& o) const {
                if(pointer_ == buffers_.size() && o.pointer_ == o.buffers_.size()) {
                    return false;
                }
                return &buffers_ != &o.buffers_ || pointer_ != o.pointer_ ||
                        pre_off_ != o.pre_off_ || cur_off_ != o.cur_off_;
            }
            ewah_bitmap_forward_iteartor& operator++() {
                ++cur_off_;
                advance_to_next_setbit();
                return *this;
            }
            ewah_bitmap_forward_iteartor operator++(int) {
                ewah_bitmap_forward_iteartor old(*this);
                +cur_off_;
                advance_to_next_setbit();
                return old;
            }
        private:
            enum { use_trailing_zeros = true };

            ewah_bitmap_forward_iteartor(const container& buf, size_t start = 0)
                : buffers_(buf), pointer_(start), pre_off_(0), cur_off_(0),
                  running_(0)
            {
                if(pointer_ < buffers_.size()) {
                    running_.data_ = buffers_[pointer_];
                    advance_to_next_setbit();
                }
            }

            bool advance_to_next_setbit() {
                if(pointer_ == buffers_.size()) {
                    return false;
                }
                if(cur_off_ < static_cast<size_t >(running_.get_running_len() * word_in_bits)) {
                    if(running_.get_running_bit()) {
                        return true;
                    }
                    cur_off_ = static_cast<size_t >(running_.get_running_len() * word_in_bits);
                }
                while(true) {
                    size_t index = static_cast<size_t >((cur_off_ - running_.get_running_len() * word_in_bits) / word_in_bits);
                    if(index >= running_.get_literal_len()) {
                        if(advance_to_next_run()) {
                            return advance_to_next_setbit();
                        }
                        else
                            return false;
                    }

                    if(use_trailing_zeros) {
                        uint32_t word_ptr = static_cast<uint32_t >((cur_off_ - running_.get_running_len() * word_in_bits) % word_in_bits);
                        uword    word_cur = static_cast<uword >(buffers_[pointer_ + 1 + index] >> word_ptr);
                        if(word_cur != 0) {
                            cur_off_ += static_cast<size_t >(detail::numberOfTrailingZeros(word_cur));
                            return true;
                        }
                        else {
                            cur_off_ += word_in_bits - word_ptr;
                        }
                    }
                    else {
                        uword word_cur = buffers_[pointer_ + 1 + index];
                        for(uint32_t word_ptr = static_cast<uint32_t >((cur_off_ - running_.get_running_len() * word_in_bits) % word_in_bits);
                            word_ptr < word_in_bits; ++word_ptr, ++cur_off_) {
                            if((word_cur & (static_cast<uword >(1) << word_ptr)) != 0)
                                return true;
                        }
                    }
                }
            }

            bool advance_to_next_run() {
                pre_off_ += cur_off_;
                cur_off_ = 0;
                pointer_ += static_cast<size_t >(1 + running_.get_literal_len());
                if(pointer_ < buffers_.size()) {
                    running_.data_ = buffers_[pointer_];
                }
                else {
                    return false;
                }
                return true;
            }

            friend class ewah_bitmap<uword >;
            const container&                    buffers_;
            size_t                              pointer_;
            size_t                              pre_off_;
            size_t                              cur_off_;
            detail::ConstRunLengthWord<uword >  running_;
        };

        template<typename uword = uint32_t >
        class ewah_bitmap_raw_iterator {
        public:
            ewah_bitmap_raw_iterator(const ewah_bitmap<uword >& p)
                : ptr_(0), buf_(&p.buffer()), rlw_((*buf_)[ptr_]) {}
            ewah_bitmap_raw_iterator(const ewah_bitmap_raw_iterator& o)
                : ptr_(o.ptr_), buf_(o.buf_), rlw_(o.rlw_) {}

            bool has_next() const { return ptr_ < buf_->size(); }
            detail::BufferedRunLengthWord<uword >& next() {
                assert(ptr_ < buf_->size());
                rlw_.read((*buf_)[ptr_]);
                ptr_ = static_cast<size_t >(ptr_ + rlw_.get_literal_len() + 1);
                return rlw_;
            }
            const uword* dirty_words() const {
                assert(ptr_ > 0);
                assert(ptr_ >= rlw_.get_literal_len());
                return &(buf_->at(static_cast<size_t >(ptr_ - rlw_.get_literal_len())));
            }
        private:
            typedef typename ewah_bitmap<uword >::container container;
            ewah_bitmap_raw_iterator();
            size_t                                  ptr_;
            const container*                        buf_;
            detail::BufferedRunLengthWord<uword >   rlw_;
        };

        class bitmap_statistics {

        };

        ////////////////////////////////////////////////////////////////////////
        template<typename uword >
        inline ewah_bitmap<uword >::ewah_bitmap()
            : buffer_(1, 0), bitlen_(0), last_(0)
        {
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::set(size_t i)
        {
            assert(i >= bitlen_);
            size_t dist = (i + word_in_bits) / word_in_bits - (bitlen_ + word_in_bits - 1) / word_in_bits;
            bitlen_ = i + 1;
            if(dist > 0) {
                if(dist > 1)
                    fast_add_stream_of_empty_words(false, dist - 1);
                add_literal_word(static_cast<uword >(static_cast<uword >(1) << (i % word_in_bits)));
                return;
            }
            detail::RunLengthWord<uword >   lastRunning(buffer_[last_]);
            if(lastRunning.get_literal_len() == 0) {
                lastRunning.set_running_len(static_cast<uword >(lastRunning.get_running_len() - 1));
                add_literal_word(static_cast<uword >(static_cast<uword >(1) << (i % word_in_bits)));
                return;
            }

            buffer_[buffer_.size() - 1] |= static_cast<uword >(static_cast<uword >(1) << (i % word_in_bits));
            if(buffer_[buffer_.size() - 1] == static_cast<uword >(~0)) {
                buffer_[buffer_.size() - 1] = 0;
                buffer_.resize(buffer_.size() - 1);
                lastRunning.set_literal_len(static_cast<uword >(lastRunning.get_literal_len() - 1));
                add_empty_word(true);
            }
        }

        template<typename uword >
        inline size_t ewah_bitmap<uword >::add(uword data, size_t bits)
        {
            bitlen_ += bits;
            if(data == 0) {
                return add_empty_word(0);
            }
            else if(data == static_cast<uword >(~0)) {
                return add_empty_word(1);
            }
            else {
                return add_literal_word(data);
            }
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::reset()
        {
            buffer_.clear();
            buffer_.push_back(0);
            bitlen_ = 0;
            last_ = 0;
        }

        template<typename uword >
        inline size_t ewah_bitmap<uword >::count() const
        {
            size_t tot = 0;
            size_t ptr = 0;
            while(ptr < buffer_.size()) {
                detail::ConstRunLengthWord<uword > rlw(buffer_[ptr]);
                if(rlw.get_running_bit()) {
                    tot += rlw.get_running_len() * word_in_bits;
                }
                ++ptr;
                for(size_t k = 0; k < rlw.get_literal_len(); ++k) {
                    assert(detail::countOnes(buffer_[ptr]) < 64);
                    tot += detail::countOnes(buffer_[ptr]);
                    ++ptr;
                }
            }
            return tot;
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::debug() const
        {
#ifndef  NDEBUG
            std::cout << "number of compressed words: " << buffer_.size() << ", bytes: " << memory() << "\n";
            size_t ptr = 0;
            while(ptr < buffer_.size()) {
                detail::ConstRunLengthWord<uword > rlw(buffer_[ptr]);
                bool b = rlw.get_running_bit();
                uword rl = rlw.get_running_len();
                uword lw = rlw.get_literal_len();
                std::cout << "pointer=" << ptr << " running bit=" << b
                          << " running length=" << rl << " literal word=" << lw << "\n";
                for(uword j = 0; j < lw; ++j) {
                    const uword& w = buffer_[ptr + j + 1];
                    std::cout << detail::to_string(w) << "\n";
                }
                ptr += lw + 1;
            }
#endif
        }

        template<typename uword >
        inline typename ewah_bitmap<uword >::bit_array ewah_bitmap<uword >::bitset() const
        {
            bit_array   ans;
            size_t  pos = 0;
            size_t  ptr = 0;
            while(ptr < buffer_.size()) {
                detail::ConstRunLengthWord<uword > rlw(buffer_[ptr]);
                if(rlw.get_running_bit()) {
                    for(size_t k = 0; k < rlw.get_running_len() * word_in_bits; ++k, ++pos) {
                        ans.push_back(pos);
                    }
                }
                else {
                    pos += rlw.get_running_len() * word_in_bits;
                }
                ++ptr;
                bool use_trailing = true;
                for(size_t k = 0; k < rlw.get_literal_len(); ++k) {
                    if(use_trailing) {
                        uword word = buffer_[ptr];
                        for(uint32_t offset = 0;  offset < word_in_bits; ++offset) {
                            if((word >> offset) == 0) break;
                            offset += static_cast<uint32_t >(detail::numberOfTrailingZeros(static_cast<uword >(word >> offset)));
                            ans.push_back(pos + offset);
                        }
                        pos += word_in_bits;
                    }
                    else {
                        for(int c = 0; c < word_in_bits; ++c, ++pos) {
                            if((buffer_[ptr] & (static_cast<uword >(1) << c)) != 0) {
                                ans.push_back(pos);
                            }
                        }
                    }
                    ++ptr;
                }
            }
            return ans;
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::_and(const ewah_bitmap<uword> &rhs, ewah_bitmap<uword> &result) const
        {
            size_t max_bitlen = bitlen() > rhs.bitlen() ? bitlen() : rhs.bitlen();
            result.reset();
            result.buffer().reserve(buffer_.size() > rhs.buffer().size() ? buffer_.size() : rhs.buffer().size());

            ewah_bitmap<uword > temp_bitmap;
            ewah_bitmap_raw_iterator<uword > i = rhs.raw_iterator();
            ewah_bitmap_raw_iterator<uword > j = this->raw_iterator();
            if(rhs.bitlen() < this->bitlen()) {
                temp_bitmap = rhs;
                temp_bitmap.padding(this->bitlen());
                i = temp_bitmap.raw_iterator();
            }
            else if(this->bitlen() < rhs.bitlen()) {
                temp_bitmap = *this;
                temp_bitmap.padding(rhs.bitlen());
                j = temp_bitmap.raw_iterator();
            }

            if(!(i.has_next() && j.has_next())) {
                result.bitlen(max_bitlen);
                return;
            }

            detail::BufferedRunLengthWord<uword >& rlwi = i.next();
            detail::BufferedRunLengthWord<uword >& rlwj = j.next();
            while(true) {
                bool i_is_prey(rlwi.size() < rlwj.size());
                detail::BufferedRunLengthWord<uword >& prey(i_is_prey ? rlwi : rlwj);
                detail::BufferedRunLengthWord<uword >& pred(i_is_prey ? rlwj : rlwi);
                if(prey.get_running_bit() == 0) {
                    uword prey_rl = prey.get_running_len();
                    pred.discard_first_words(prey_rl);
                    prey.discard_first_words(prey_rl);
                    result.add_stream_of_empty_words(0, static_cast<size_t >(prey_rl));
                }
                else {
                    uword pred_rl = pred.get_running_len();
                    uword prey_rl = prey.get_running_len();
                    uword discard = (pred_rl >= prey_rl) ? prey_rl : pred_rl;
                    result.add_stream_of_empty_words(pred.get_running_bit(), static_cast<size_t >(discard));
                    if(prey_rl - discard > 0) {
                        const uword* pred_word = i_is_prey ? j.dirty_words() : i.dirty_words();
                        result.add_stream_of_dirty_words(pred_word, static_cast<size_t >(prey_rl - discard));
                    }
                    pred.discard_first_words(prey_rl);
                    prey.discard_first_words(prey_rl);
                }
                uword pred_rl = pred.get_running_len();
                if(pred_rl > 0) {
                    if(pred.get_running_bit() == 0) {
                        uword nbre_dirty_prey = prey.get_literal_len();
                        uword discard = (pred_rl >= nbre_dirty_prey) ? nbre_dirty_prey : pred_rl;
                        pred.discard_first_words(discard);
                        prey.discard_first_words(discard);
                        result.add_stream_of_empty_words(0, static_cast<size_t >(discard));
                    }
                    else {
                        uword nbre_dirty_prey = prey.get_literal_len();
                        uword discard = (pred_rl >= nbre_dirty_prey) ? nbre_dirty_prey : pred_rl;
                        if(discard > 0) {
                            const uword* prey_word = i_is_prey ? i.dirty_words() : j.dirty_words();
                            result.add_stream_of_dirty_words(prey_word, discard);
                            pred.discard_first_words(discard);
                            prey.discard_first_words(discard);
                        }
                    }
                }
                assert(prey.get_running_len() == 0);
                uword nbre_dirty_prey = prey.get_literal_len();
                if(nbre_dirty_prey > 0) {
                    assert(pred.get_running_len() == 0);
                    const uword* i_dirty = i.dirty_words();
                    const uword* j_dirty = j.dirty_words();
                    for(uword k = 0; k < nbre_dirty_prey; ++k) {
                        result.add(i_dirty[k] & j_dirty[k]);
                    }
                    pred.discard_first_words(nbre_dirty_prey);
                }
                if(i_is_prey) {
                    if(!i.has_next())
                        break;
                    rlwi = i.next();
                }
                else {
                    if(!j.has_next())
                        break;
                    rlwj = j.next();
                }
            }
            result.bitlen(max_bitlen);
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::_or(const ewah_bitmap<uword> &rhs, ewah_bitmap<uword> &result) const
        {
            size_t max_bitlen = bitlen() > rhs.bitlen() ? bitlen() : rhs.bitlen();
            result.reset();
            result.buffer().reserve(buffer_.size() > rhs.buffer().size() ? buffer_.size() : rhs.buffer().size());

            ewah_bitmap<uword > temp_bitmap;
            ewah_bitmap_raw_iterator<uword > i = rhs.raw_iterator();
            ewah_bitmap_raw_iterator<uword > j = this->raw_iterator();
            if(rhs.bitlen() < this->bitlen()) {
                temp_bitmap = rhs;
                temp_bitmap.padding(this->bitlen());
                i = temp_bitmap.raw_iterator();
            }
            else if(this->bitlen() < rhs.bitlen()) {
                temp_bitmap = *this;
                temp_bitmap.padding(rhs.bitlen());
                j = temp_bitmap.raw_iterator();
            }

            if(!(i.has_next() && j.has_next())) {
                result.bitlen(max_bitlen);
                return;
            }

            detail::BufferedRunLengthWord<uword >& rlwi = i.next();
            detail::BufferedRunLengthWord<uword >& rlwj = j.next();
            while(true) {
                bool i_is_prey = rlwi.size() < rlwj.size();
                detail::BufferedRunLengthWord<uword >& prey(i_is_prey ? rlwi : rlwj);
                detail::BufferedRunLengthWord<uword >& pred(i_is_prey ? rlwj : rlwi);
                if(prey.get_running_bit() == 0) {
                    uword pred_rl = pred.get_running_len();
                    uword prey_rl = prey.get_running_len();
                    if(pred_rl >= prey_rl) {
                        uword discard = prey_rl;
                        result.add_stream_of_empty_words(pred.get_running_bit(), static_cast<size_t >(discard));
                    }
                    else {
                        uword discard = pred_rl;
                        result.add_stream_of_empty_words(pred.get_running_bit(), static_cast<size_t >(discard));
                        if(prey_rl - discard > 0) {
                            const uword* pred_word = i_is_prey ? j.dirty_words() : i.dirty_words();
                            result.add_stream_of_dirty_words(pred_word, static_cast<size_t >(prey_rl - discard));
                        }
                    }
                    pred.discard_first_words(prey_rl);
                    prey.discard_first_words(prey_rl);
                }
                else {
                    uword prey_rl = prey.get_running_len();
                    pred.discard_first_words(prey_rl);
                    prey.discard_first_words(prey_rl);
                    result.add_stream_of_empty_words(1, static_cast<size_t >(prey_rl));
                }
                uword pred_rl = pred.get_running_len();
                if(pred_rl > 0) {
                    if(pred.get_running_bit() == 0) {
                        uword nbre_dirty_prey = prey.get_literal_len();
                        uword discard = (pred_rl >= nbre_dirty_prey) ? nbre_dirty_prey : pred_rl;
                        if(discard > 0) {
                            const uword* prey_word = i_is_prey ? i.dirty_words() : j.dirty_words();
                            result.add_stream_of_dirty_words(prey_word, static_cast<size_t >(discard));
                            pred.discard_first_words(discard);
                            prey.discard_first_words(discard);
                        }
                    }
                    else {
                        uword nbre_dirty_prey = prey.get_literal_len();
                        uword discard = (pred_rl >= nbre_dirty_prey) ? nbre_dirty_prey : pred_rl;
                        pred.discard_first_words(discard);
                        prey.discard_first_words(discard);
                        result.add_stream_of_empty_words(1, static_cast<size_t >(discard));
                    }
                }
                assert(prey.get_running_len() == 0);
                uword nbre_dirty_prey = prey.get_literal_len();
                if(nbre_dirty_prey > 0) {
                    assert(pred.get_running_len() == 0);
                    const uword* idirty = i.dirty_words();
                    const uword* jdirty = j.dirty_words();
                    for(uword k = 0; k < nbre_dirty_prey; ++k) {
                        result.add(idirty[k] | jdirty[k]);
                    }
                    pred.discard_first_words(nbre_dirty_prey);
                }
                if(i_is_prey) {
                    if(!i.has_next())
                        break;
                    rlwi = i.next();
                }
                else {
                    if(!j.has_next())
                        break;
                    rlwj = j.next();
                }
            }
            result.bitlen(max_bitlen);
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::_not(ewah_bitmap<uword> &result) const
        {
            result.reset();
            result.buffer().reserve(buffer_.size());
            ewah_bitmap_raw_iterator<uword > i = this->raw_iterator();
            while(i.has_next()) {
                detail::BufferedRunLengthWord<uword >& rlw = i.next();
                result.add_stream_of_empty_words(!rlw.get_running_bit(), rlw.get_running_len());
                if(rlw.get_literal_len() > 0) {
                    const uword* word = i.dirty_words();
                    for(size_t k = 0; k < rlw.get_literal_len(); ++k) {
                        result.add_literal_word(~word[k]);
                    }
                }
            }
            result.bitlen(this->bitlen());
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::_not()
        {
            size_t ptr = 0;
            while(ptr < buffer_.size()) {
                detail::RunLengthWord<uword > rlw(buffer_[ptr]);
                if(rlw.get_running_bit())
                    rlw.set_running_bit(false);
                else
                    rlw.set_running_bit(true);
                ++ptr;

                for(size_t k = 0; k < rlw.get_literal_len(); ++k) {
                    buffer_[ptr] = ~buffer_[ptr];
                    ++ptr;
                }
            }
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::append(const ewah_bitmap<uword> &rhs)
        {
            assert(bitlen_ % word_in_bits == 0);
            if(bitlen_ % word_in_bits == 0) {
                bitlen_ += rhs.bitlen();
                detail::ConstRunLengthWord<uword > rlw(buffer_[last_]);
                if((rlw.get_running_len() == 0) && (rlw.get_literal_len() == 0)) {
                    assert(last_ == buffer_.size() - 1);
                    last_ = rhs.last_ + buffer_.size() - 1;
                    buffer_.resize(buffer_.size() - 1);
                    buffer_.insert(buffer_.end(), rhs.buffer().begin(), rhs.buffer().end());
                }
                else {
                    last_ = rhs.last_ + buffer_.size();
                    buffer_.insert(buffer_.end(), rhs.buffer().begin(), rhs.buffer().end());
                }
            }
            else {
                assert(false);
            }
        }

        template<typename uword >
        inline size_t ewah_bitmap<uword >::add_literal_word(uword data)
        {
            detail::RunLengthWord<uword > lastRunning(buffer_[last_]);
            size_t number = lastRunning.get_literal_len();
            if(number >= detail::RunLengthWord<uword >::LargestLiteralCount) {
                buffer_.push_back(0);
                last_ = buffer_.size() - 1;
                detail::RunLengthWord<uword > lastRunning2(buffer_[last_]);
                lastRunning2.set_literal_len(1);
                buffer_.push_back(data);
                return 2;
            }
            lastRunning.set_literal_len(static_cast<uword >(number + 1));
            assert(lastRunning.get_literal_len() == number + 1);
            buffer_.push_back(data);
            return 1;
        }

        template<typename uword >
        inline size_t ewah_bitmap<uword >::add_empty_word(bool v)
        {
            detail::RunLengthWord<uword > lastRunning(buffer_[last_]);
            bool not_literal_word = (lastRunning.get_literal_len() == 0);
            uword run_len = lastRunning.get_running_len();
            if(not_literal_word && run_len == 0) {
                lastRunning.set_running_bit(v);
                assert(lastRunning.get_running_bit() == v);
            }
            if(not_literal_word && lastRunning.get_running_bit() == v &&
                run_len < detail::RunLengthWord<uword >::LargestRunningLengthCount) {
                lastRunning.set_running_len(static_cast<uword >(run_len + 1));
                assert(lastRunning.get_running_len() == run_len + 1);
                return 0;
            }
            else {
                buffer_.push_back(0);
                last_ = buffer_.size() - 1;
                detail::RunLengthWord<uword > lastRunning2(buffer_[last_]);
                assert(lastRunning2.get_running_len() == 0);
                assert(lastRunning2.get_running_bit() == 0);
                assert(lastRunning2.get_literal_len() == 0);
                lastRunning2.set_running_bit(v);
                assert(lastRunning2.get_running_bit() == v);
                lastRunning2.set_running_len(1);
                assert(lastRunning2.get_running_len() == 1);
                assert(lastRunning2.get_literal_len() == 0);
                return 1;
            }
        }

        template<typename uword >
        inline size_t ewah_bitmap<uword>::padding(size_t s)
        {
            assert(bitlen_ <= s);
            size_t missing_bit = s - bitlen_;
            size_t words_added = add_stream_of_empty_words(
                                    0,
                                    missing_bit / word_in_bits + ((missing_bit % word_in_bits != 0) ? 1 : 0));
            assert(bitlen_ >= s);
            assert(bitlen_ <= s + word_in_bits);
            bitlen_ = s;
            return words_added;
        }

        template<typename uword >
        inline size_t ewah_bitmap<uword >::add_stream_of_empty_words(bool v, size_t number)
        {
            if(number == 0) {
                return 0;
            }
            bitlen_ += number * word_in_bits;
            size_t words_added = 0;
            if((detail::RunLengthWord<uword >::get_running_bit(buffer_[last_]) != v) &&
                    (detail::RunLengthWord<uword >::size(buffer_[last_]) == 0)) {
                detail::RunLengthWord<uword >::set_running_bit(buffer_[last_], v);
            }
            else if((detail::RunLengthWord<uword >::get_literal_len(buffer_[last_]) != 0) ||
                    (detail::RunLengthWord<uword >::get_running_bit(buffer_[last_]) != v)) {
                buffer_.push_back(0);
                ++words_added;
                last_ = buffer_.size() - 1;
                if(v) {
                    detail::RunLengthWord<uword >::set_running_bit(buffer_[last_], v);
                }
            }

            size_t run_len = detail::RunLengthWord<uword >::get_running_len(buffer_[last_]);
            size_t can_add = number < static_cast<uword >(detail::RunLengthWord<uword >::LargestRunningLengthCount - run_len) ?
                                static_cast<uword >(number) :
                                static_cast<uword >(detail::RunLengthWord<uword >::LargestRunningLengthCount - run_len);

            detail::RunLengthWord<uword >::set_running_len(buffer_[last_], static_cast<uword >(run_len + can_add));
            number -= static_cast<uword >(can_add);

            while(number >= detail::RunLengthWord<uword >::LargestRunningLengthCount) {
                buffer_.push_back(0);
                ++words_added;
                last_ = buffer_.size() - 1;
                if(v)
                    detail::RunLengthWord<uword >::set_running_bit(buffer_[last_], v);

                detail::RunLengthWord<uword >::set_running_len(buffer_[last_],
                                                               detail::RunLengthWord<uword >::LargestRunningLengthCount);
                number -= static_cast<uword >(detail::RunLengthWord<uword >::LargestRunningLengthCount);
            }

            if(number > 0) {
                buffer_.push_back(0);
                ++words_added;
                last_ = buffer_.size() - 1;
                if(v)
                    detail::RunLengthWord<uword >::set_running_bit(buffer_[last_], v);
                detail::RunLengthWord<uword >::set_running_len(buffer_[last_], number);
            }
            return words_added;
        }

        template<typename uword >
        inline size_t ewah_bitmap<uword >::add_stream_of_dirty_words(const uword *v, size_t number)
        {
            if(number == 0)
                return 0;
            detail::RunLengthWord<uword > lastRunning(buffer_[last_]);
            uword literal_len = lastRunning.get_literal_len();
            assert(detail::RunLengthWord<uword >::LargestLiteralCount >= literal_len);

            size_t can_add = number < static_cast<uword >(detail::RunLengthWord<uword >::LargestLiteralCount - literal_len) ?
                                 number : static_cast<size_t >(detail::RunLengthWord<uword >::LargestLiteralCount - literal_len);
            assert(can_add + literal_len <= detail::RunLengthWord<uword >::LargestLiteralCount);

            lastRunning.set_literal_len(static_cast<uword >(literal_len + can_add));

            assert(lastRunning.get_literal_len() == literal_len + can_add);

            size_t needed = number - can_add;
            size_t oldlen = buffer_.size();
            buffer_.resize(oldlen + can_add);
            memcpy(&buffer_[oldlen], v, can_add * sizeof(uword));
            size_t word_added = can_add;

            if(needed > 0) {
                buffer_.push_back(0);
                last_ = buffer_.size() - 1;
                ++word_added;
                word_added += add_stream_of_dirty_words(v + can_add, needed);
            }
            assert(word_added >= number);
            return word_added;
        }

        template<typename uword >
        inline void ewah_bitmap<uword >::fast_add_stream_of_empty_words(bool v, size_t number)
        {
            if((detail::RunLengthWord<uword >::get_running_bit(buffer_[last_]) != v) &&
                    detail::RunLengthWord<uword >::size(buffer_[last_]) == 0) {
                detail::RunLengthWord<uword >::set_running_bit(buffer_[last_], v);
            }
            else if(detail::RunLengthWord<uword >::get_literal_len(buffer_[last_] != 0) ||
                    detail::RunLengthWord<uword >::get_running_bit(buffer_[last_] != v)) {
                buffer_.push_back(0);
                last_ = buffer_.size() - 1;
                if(v) {
                    detail::RunLengthWord<uword >::set_running_bit(buffer_[last_], v);
                }
            }
            size_t run_len = detail::RunLengthWord<uword >::get_running_len(buffer_[last_]);
            size_t can_add = number < static_cast<size_t >(detail::RunLengthWord<uword >::LargestRunningLengthCount - run_len) ?
                                static_cast<uword >(number) : static_cast<uword >(detail::RunLengthWord<uword >::LargestRunningLengthCount - run_len);
            detail::RunLengthWord<uword >::set_running_len(buffer_[last_], static_cast<uword >(run_len + can_add));
            number -= static_cast<size_t >(can_add);

            while(number >= detail::RunLengthWord<uword >::LargestRunningLengthCount) {
                buffer_.push_back(0);
                last_ = buffer_.size() - 1;
                if(v)
                    detail::RunLengthWord<uword >::set_running_bit(buffer_[last_], v);
                detail::RunLengthWord<uword >::set_running_len(buffer_[last_], detail::RunLengthWord<uword >::LargestRunningLengthCount);
                number -= static_cast<size_t >(detail::RunLengthWord<uword >::LargestRunningLengthCount);
            }
            if(number > 0) {
                buffer_.push_back(0);
                last_ = buffer_.size() - 1;
                if(v)
                    detail::RunLengthWord<uword >::set_running_bit(buffer_[last_], v);
                detail::RunLengthWord<uword >::set_running_len(buffer_[last_], static_cast<uword >(number));
            }
        }

    }
}

#endif
