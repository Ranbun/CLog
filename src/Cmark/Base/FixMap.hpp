#ifndef CMARK_FIXMAP_H_
#define CMARK_FIXMAP_H_

#include <unordered_map>
#include <queue>
#include <memory>
#include <QPixmap>

namespace CM
{
    template<typename T1, typename T2>
    class FixMap
    {
    public:
        FixMap() = default;
        ~FixMap() = default;

        FixMap(const FixMap&) = delete;
        FixMap(const FixMap&&) = delete;

        FixMap& operator=(const FixMap&) = delete;
        FixMap& operator=(const FixMap&&) = delete;

    protected:
        int m_MaxSize = 20;

    };

    template<typename T>
    class FixMap<size_t, T>
    {
    public:
        FixMap() = default;
        ~FixMap()
        {
            m_Maps.clear();
        }

        FixMap(const FixMap&) = delete;
        FixMap(const FixMap&&) = delete;

        FixMap& operator=(const FixMap&) = delete;
        FixMap& operator=(const FixMap&&) = delete;


        void insert(const std::pair<size_t, T>& d)
        {
            if (m_Maps.size() > m_MaxSize)
            {
                const auto front = m_IndexCodeQueue.front();
                m_Maps.erase(front);
                m_IndexCodeQueue.pop();
            }

            m_IndexCodeQueue.emplace(d.first);
            m_Maps.insert(d);
        }

        void insert(const size_t key, const T& value)
        {
            insert({ key,value });
        }


        T getPixmap(const size_t indexCode)
        {
            if (m_Maps.count(indexCode))
            {
                return m_Maps.at(indexCode);
            }
            return nullptr;
        }


        void remove(const size_t indexCode)
        {
            if (m_Maps.count(indexCode))
            {
                m_Maps.erase(indexCode);

                std::queue<size_t> tempQueue;
                tempQueue.swap(m_IndexCodeQueue);

                while (!tempQueue.empty())
                {
                    if (auto f = tempQueue.front(); f != indexCode)
                    {
                        m_IndexCodeQueue.emplace(f);
                    }
                    else
                    {
                        tempQueue.pop();
                    }
                }
            }
        }

        void clear()
        {
            while (!m_IndexCodeQueue.empty())
            {
                m_IndexCodeQueue.pop();
            }
            m_Maps.clear();
        }

        std::unordered_map<size_t, std::shared_ptr<QPixmap>>::iterator begin(){return m_Maps.begin();}
        std::unordered_map<size_t, std::shared_ptr<QPixmap>>::iterator end(){return m_Maps.end();}

    private:
        uint64_t m_MaxSize = 20;
        std::queue<size_t> m_IndexCodeQueue;
        std::unordered_map<size_t, T> m_Maps;
    };

}


#endif // !CMARK_FIXMAP_H_
