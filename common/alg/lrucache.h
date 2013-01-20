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
         * ģ���������Ĳ���
         * +K       cache��key������
         * +D       cache�е���������
         * +S       ����cache�����ĺ���������ͨ�������ú�����ȷ��cache���ڴ�ʹ���ǰ���
         *          ������������ǻ���Ĵ�С��Ĭ���ǻ��������
         * +F       ��cache��ʧЧ�����ݱ��浽�ⲿ�洢�ĺ�����Ĭ�ϲ����档
         * +L       �����cache��û���ҵ�ָ����key������øú����ӻ�ȡ��
         */

        // ������������cache����Ĵ�С
        template<typename K, typename D >
        struct CountSizer {
            std::size_t operator()(const K&, const D&) const {
                return 1;
            }
        };

        // ������ʧЧ������
        template<typename K, typename D >
        struct NullKeeper {
            void operator()(const K&, const D& ) const {
                return;
            }
        };

        // �����ⲿ��ȡ����
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

            // ����һ��LRU����
            explicit LruCache(size_t capacity);
            ~LruCache();

            // ��ջ���
            void clear();
            // �ж�ĳ��key�Ƿ���cache��
            bool exist(const K& key) const;
            // ����key��cache��ɾ��һ��Ԫ��
            void erase(const K& key);
            // ����keyˢ�����ݣ���֤LRU
            void touch(const K& key);
            // ����key��ȡ����
            D*   fetch(const K& key, bool touch);
            // ����key��ȡ����
            bool fetch(const K& key, D& data, bool touch);
            // ��������
            void input(const K& key, const D& data);
            // �г���ǰcache�����е�key
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
            // ���ҵ��Ľڵ��Ƶ�ǰ��
            list_.splice(list_.begin(), list_, it->second);
        }

        template<typename K, typename D, typename S, typename F, typename L >
        inline D* LruCache<K, D, S, F, L >::fetch(const K& key, bool touch)
        {
            use_++;
            MapIter mi = idex_.find(key);
            // ���û���ҵ�����ô�����ⲿ����
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
            // ɾ�����е�ֵ
            MapIter mi = fresh(key);
            if(mi != idex_.end()) {
                erase(mi);
            }

            list_.push_front(std::make_pair(key, data));
            ListIter li = list_.begin();

            idex_.insert(std::make_pair(key, li));
            cur_ += S()(key, data);

            // ��������˻�����޶ȣ���ɾ���ϵ�����
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
