#include <grafkit/utils/tree.hpp>
#include <gtest/gtest.h>

using namespace Grafkit::Utils;

TEST(TreeTest, AddRootNode)
{
	Tree<int> tree;
	auto root = tree.Add(1);
	ASSERT_NE(root, nullptr);
	ASSERT_EQ(root->m_data, 1);
}

TEST(TreeTest, AddChildNode)
{
	Tree<int> tree;
	auto root = tree.Add(1);
	auto child = tree.Add(2, root);
	ASSERT_NE(child, nullptr);
	ASSERT_EQ(child->m_data, 2);
	ASSERT_EQ(root->m_children.size(), 1);
	ASSERT_EQ(root->m_children[0], child);
}

TEST(TreeTest, RemoveLeafNode)
{
	Tree<int> tree;
	auto root = tree.Add(1);
	auto child = tree.Add(2, root);
	tree.Remove(child);
	ASSERT_EQ(root->m_children.size(), 0);
}

TEST(TreeTest, RemoveNonLeafNode)
{
	Tree<int> tree;
	auto root = tree.Add(1);
	auto child = tree.Add(2, root);
	auto grandchild = tree.Add(3, child);
	tree.Remove(child);
	ASSERT_EQ(root->m_children.size(), 0);
}

TEST(TreeTest, TopologicalSort)
{
	Tree<int> tree;
	auto root = tree.Add(1);
	auto child1 = tree.Add(2, root);
	auto child2 = tree.Add(3, root);
	auto grandchild = tree.Add(4, child1);

	auto sortedList = tree.TopologicalSort();
	std::vector<int> sortedData;
	for (const auto &node : sortedList)
	{
		sortedData.push_back(node->m_data);
	}

	std::vector<int> expectedOrder = {1, 2, 4, 3};
	ASSERT_EQ(sortedData, expectedOrder);
}
