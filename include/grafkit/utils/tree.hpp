#ifndef GRAFKIT_UTILS_TREE_HPP
#define GRAFKIT_UTILS_TREE_HPP

#include <algorithm>
#include <list>
#include <memory>
#include <stack>
#include <unordered_set>
#include <vector>

namespace Grafkit::Utils
{

	template <typename T>
	class Node
	{
	public:
		T m_data;
		std::vector<std::shared_ptr<Node<T>>> m_children;

		Node(const T &data)
			: m_data(data)
		{
		}
	};

	template <typename T>
	class Tree
	{
	public:
		Tree()
			: m_root(nullptr)
		{
		}

		std::shared_ptr<Node<T>> Add(const T &data, std::shared_ptr<Node<T>> parent = nullptr)
		{
			auto newNode = std::make_shared<Node<T>>(data);
			if (!parent)
			{
				if (!m_root)
				{
					m_root = newNode;
				}
				else
				{
					m_root->m_children.push_back(newNode);
				}
			}
			else
			{
				parent->m_children.push_back(newNode);
			}
			return newNode;
		}

		void Remove(std::shared_ptr<Node<T>> node)
		{
			if (!node)
			{
				return;
			}
			if (node == m_root)
			{
				m_root.reset();
			}
			else
			{
				RemoveNodes(m_root, node);
			}
		}

		std::list<std::shared_ptr<Node<T>>> TopologicalSort()
		{
			std::list<std::shared_ptr<Node<T>>> sortedList;
			std::stack<std::shared_ptr<Node<T>>> stack;
			std::unordered_set<std::shared_ptr<Node<T>>> visited;

			if (m_root)
			{
				stack.push(m_root);
			}

			while (!stack.empty())
			{
				auto current = stack.top();
				stack.pop();

				if (visited.find(current) == visited.end())
				{
					visited.insert(current);
					sortedList.push_front(current);

					for (auto &child : current->m_children)
					{
						stack.push(child);
					}
				}
			}

			return sortedList;
		}

	private:
		std::shared_ptr<Node<T>> m_root;

		void RemoveNodes(std::shared_ptr<Node<T>> parent, std::shared_ptr<Node<T>> node)
		{
			if (!parent)
			{
				return;
			}

			std::stack<std::shared_ptr<Node<T>>> stack;
			stack.push(parent);

			while (!stack.empty())
			{
				auto current = stack.top();
				stack.pop();

				auto it = std::find(current->m_children.begin(), current->m_children.end(), node);
				if (it != current->m_children.end())
				{
					current->m_children.erase(it);
					return;
				}

				for (auto &child : current->m_children)
				{
					stack.push(child);
				}
			}
		}
	};

} // namespace Grafkit::Utils

#endif // GRAFKIT_UTILS_TREE_HPP
