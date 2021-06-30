#ifndef THREAD_SAFE_MAP_HPP
#define THREAD_SAFE_MAP_HPP

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>

/**
 * 标准库关联容器的接口不适合并发访问
 * 最大的问题在于迭代器，其他线程删除元素时导致迭代器失效
 * 因此接口设计就要跳过迭代器
 *
 * 为了使用细粒度锁，就不应该使用标准库容器
 * 可选的关联容器数据结构有三种
 *      一是二叉树（如红黑树）
 *          但每次查找修改都要从访问根节点开始，也就表示根节点需要上锁，
 *          尽管沿着树向下访问节点时会解锁，
 *          但这个比起覆盖整个数据结构的单个锁好不了多少
 *      第二种方式是有序数组，这比二叉树还差，
 *          因为无法提前得知一个给定的值应该放在哪，
 *          于是同样需要一个覆盖整个数组的锁
 *      第三种方式是哈希表。假如有一个固定数量的桶，
 *          一个key属于哪个桶就取决于key的属性和哈希函数，
 *          这意味着可以安全地分开锁住每个桶。
 *          如果复用一个支持多个读单个写的mutex，
 *          就能将并发度提高相当于桶数量的倍数
 */

template<typename Key, typename Value, typename Hash = std::hash<Key>>
class thread_safe_map {
	struct inner_bucket;

public:
	using key_type		  = Key;
	using const_key_ref	  = const key_type&;
	using value_type	  = Value;
	using value_ref		  = value_type&;
	using const_value_ref = const value_type&;
	using pair_type		  = std::pair<const key_type, value_type>;

	using hash_type		  = Hash;

	using unique_bucket	  = std::unique_ptr<inner_bucket>;

	// 桶的默认数量为19（一般用x%桶数决定放置x的桶的索引，桶数为质数可以使得桶分布均匀）
	explicit thread_safe_map(size_t bucket_size = 19, hash_type hash = hash_type{})
		: buckets(bucket_size),
		  hash(hash) {
		for (auto& bucket: buckets) {
			bucket.reset(new inner_bucket);
		}
	}

	thread_safe_map(const thread_safe_map&) = delete;
	thread_safe_map& operator=(const thread_safe_map&) = delete;

	auto			 get_value(const_key_ref key, const_value_ref default_value = {}) const -> value_type {
		return get_bucket(key).get_value(key, default_value);
	}

	auto add_or_update(const_key_ref key, const_value_ref value) -> void {
		get_bucket(key).add_or_update(key, value);
	}

	auto erase(const_key_ref key) -> void {
		get_bucket(key).erase(key);
	}

	auto to_map() const -> std::map<key_type, value_type> {
		std::vector<std::unique_lock<std::shared_mutex>> lock;
		for (auto& bucket: buckets) {
			lock.emplace_back(bucket->mutex);
		}

		std::map<key_type, value_type> ret;
		for (auto& bucket: buckets) {
			for (auto& kv: bucket->data) {
				ret.insert(kv);
			}
		}
		return ret;
	}

private:
	std::vector<unique_bucket> buckets;
	hash_type				   hash;

	auto					   get_bucket(const_key_ref key) const -> inner_bucket& {
		  // 桶数固定因此可以无锁调用
		  return buckets[hash(key) & buckets.size()].operator*();
	  }

	struct inner_bucket {
		std::list<pair_type>	  data;
		// 每个桶都用这个锁保护
		mutable std::shared_mutex mutex;

		auto					  get_value(const_key_ref key, const_value_ref default_value) const -> value_type {
			 // 读，共享
			 std::shared_lock<std::shared_mutex> lock(mutex);
			 auto								 it = std::find_if(data.cbegin(), data.cend(), [&key](const pair_type& kv) { return kv.first == key; });
			 return it == data.cend() ? default_value : it->second;
		 }

		auto add_or_update(const_key_ref key, const_value_ref value) -> void {
			// 写，独占
			std::unique_lock<std::shared_mutex> lock(mutex);
			auto								it = std::find_if(data.begin(), data.end(), [&key](const pair_type& kv) { return kv.first == key; });
			if (it == data.end()) {
				data.emplace_back(key, value);
			} else {
				it->second = value;
			}
		}

		auto erase(const_key_ref key) -> void {
			// 写，独占
			std::unique_lock<std::shared_mutex> lock(mutex);
			auto								it = std::find_if(data.begin(), data.end(), [&key](const pair_type& kv) { return kv.first == key; });
			if (it != data.end()) {
				data.erase(it);
			}
		}
	};
};

#endif//THREAD_SAFE_MAP_HPP
