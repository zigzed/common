/** Copyright (C) 2013 wilburlang@gmail.com
 */
#ifndef CXX_ALG_LRUCACHE_H
#define CXX_ALG_LRUCACHE_H
#include <map>
#include <list>
#include <vector>

namespace cxx {
    namespace alg {

        /**
         * @brief a C++ template for LRU cache algorithm
         *
         * 模板包含下面的参数
         * +K       cache的key的类型
         * +D       cache中的数据类型
         * +S       计算cache容量的函数。可以通过调整该函数来确定cache的内存使用是按照
         *          缓存的条数还是缓存的大小。默认是缓存的条数
         * +F       将cache中失效的数据保存到外部存储的函数。默认不保存。
         * +L       如果在cache中没有找到指定的key，则调用该函数从获取。
         */

        // 按照条数返回cache缓存的大小
        template<typename K, typename D >
        struct CountSizer {
            std::size_t operator()(const K&, const D&) const {
                return 1;
            }
        };

        // 不保存失效的数据
        template<typename K, typename D >
        struct NullKeeper {
            void operator()(const K&, const D& ) const {
                return;
            }
        };

        // 不从外部获取数据
        template<typename K, typename D >
        struct NullLoader {
            bool operator()(const K&, D& ) {
                return false;
            }
        };

        template<typename K, typename D,
                 typename S = CountSizer<K, D >,
                 typename F = NullKeeper<K, D >,
                 typename L = NullLoader<K, D >
                 >
        class LruCache {
        public:
            typedef std::size_t                         size_t;
            typedef std::list<std::pair<K, D > >        List;
            typedef typename List::iterator             ListIter;
            typedef typename List::const_iterator       ListConstIter;
            typedef std::vector<K >                     KeyList;
            typedef typename KeyList::iterator          KeyListIter;
            typedef typename KeyList::const_iterator    KeyListConstIter;
            typedef std::map<K, ListIter >              Map;
            typedef std::pair<K, ListIter >             Pair;
            typedef typename Map::iterator              MapIter;
            typedef typename Map::const_iterator        MapConstIter;

            // 构造一个LRU缓存
            explicit LruCache(size_t capacity);
            ~LruCache();

            // 清空缓存
            void clear();
            // 判断某个key是否在cache中
            bool exist(const K& key) const;
            // 按照key从cache中删除一个元素
            void erase(const K& key);
            // 按照key刷新数据，保证LRU
            void touch(const K& key);
            // 按照key获取数据
            D*   fetch(const K& key, bool touch);
            // 按照key获取数据
            bool fetch(const K& key, D& data, bool touch);
            // 插入数据
            void input(const K& key, const D& data);
            // 列出当前cache中所有的key
            bool lists(KeyList& keys) const;
            float ratio() const;

            inline size_t size() const { return cur_; }
            inline size_t capacity() const { return max_; }
        private:
            MapIter fresh(const K& k);
            MapIter fresh(MapIter it);
            void    erase(MapIter it);

            List    list_;
            Map     idex_;
            size_t  max_;
            size_t  cur_;
            size_t  hit_;
            size_t  use_;
        };

        template<typename K, typename D, typename S, typename F, typename L >
        inline LruCache<K, D, S, F, L >::LruCache(LruCache<K, D, S, F, L >::size_t size)
            : max_(size), cur_(0), hit_(0), use_(0)
        {
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline LruCache<K, D, S, F, L >::~LruCache()
        {
            clear();
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline bool LruCache<K, D, S, F, L >::exist(const K& key) const
        {
            return idex_.find(key) != idex_.end();
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline void LruCache<K, D, S, F, L >::erase(const K& key)
        {
            MapIter mi = idex_.find(key);
            if(mi != idex_.end()) {
                erase(mi);
            }
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline void LruCache<K, D, S, F, L >::erase(LruCache<K, D, S, F, L >::MapIter it)
        {
            cur_ -= S()(it->first, it->second->second);
            list_.erase(it->second);
            idex_.erase(it);
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline void LruCache<K, D, S, F, L >::touch(const K& key)
        {
            fresh(key);
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline typename LruCache<K, D, S, F, L >::MapIter LruCache<K, D, S, F, L >::fresh(const K& key)
        {
            MapIter mi = idex_.find(key);
            if(mi != idex_.end()) {
                fresh(mi);
            }
            return mi;
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline typename LruCache<K, D, S, F, L >::MapIter LruCache<K, D, S, F, L >::fresh(LruCache<K, D, S, F, L >::MapIter it)
        {
            // 将找到的节点移到前面
            list_.splice(list_.begin(), list_, it->second);
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline D* LruCache<K, D, S, F, L >::fetch(const K& key, bool touch)
        {
            use_++;
            MapIter mi = idex_.find(key);
            // 如果没有找到，那么加载外部数据
            if(mi == idex_.end()) {
                D tmp;
                if(L()(key, tmp)) {
                    hit_--;
                    input(key, tmp);
                    mi = idex_.find(key);
                }
            }
            if(mi != idex_.end()) {
                hit_++;
                D* tmp = &(mi->second->second);
                if(touch) {
                    fresh(mi);
                }
                return tmp;
            }
            return 0;
        }

//        template<typename K, typename D, typename S, typename F, typename L >
//        inline D LruCache<K, D, S, F, L >::fetch(const K& key, bool touch)
//        {
//            MapIter mi = idex_.find(key);
//            if(mi != idex_.end()) {
//                D tmp = mi->second->second;
//                if(touch) {
//                    fresh(mi);
//                }
//                return tmp;
//            }
//            return D();
//        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline bool LruCache<K, D, S, F, L >::fetch(const K& key, D& data, bool touch)
        {
            D* tmp = fetch(key, touch);
            if(tmp != 0) {
                data = *tmp;
                return true;
            }
            return false;
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline void LruCache<K, D, S, F, L >::input(const K& key, const D& data)
        {
            // 删除已有的值
            MapIter mi = fresh(key);
            if(mi != idex_.end()) {
                erase(mi);
            }

            list_.push_front(std::make_pair(key, data));
            ListIter li = list_.begin();

            idex_.insert(std::make_pair(key, li));
            cur_ += S()(key, data);

            // 如果超过了缓存的限度，则删除老的数据
            while(cur_ > max_) {
                li = list_.end();
                --li;
                F()(li->first, li->second);
                erase(li->first);
            }
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline bool LruCache<K, D, S, F, L >::lists(LruCache<K, D, S, F, L >::KeyList& keys) const
        {
            for(ListIter li = list_.begin(); li != list_.end(); ++li) {
                keys.push_back(li->first);
            }
            return !keys.empty();
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline void LruCache<K, D, S, F, L >::clear()
        {
            list_.clear();
            idex_.clear();
            cur_ = 0;
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline float LruCache<K, D, S, F, L >::ratio() const
        {
            if(use_ != 0) {
                return (float)hit_/(float)use_;
            }
            return 0;
        }
    }
}

#endif
