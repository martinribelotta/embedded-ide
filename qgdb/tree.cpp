#include "tree.h"
#include <QList>
#include "log.h"
#include "util.h"
#include <assert.h>


void TreeNode::copy(const TreeNode &other)
{
	// Remove all children
	removeAll();

	// Set name and data
	m_name = other.m_name;
	m_data = other.m_data;

	// Copy all children
	for(int i = 0;i < other.m_children.size();i++) {
		TreeNode* otherNode = other.m_children[i];
		TreeNode* thisNode = new TreeNode;
		thisNode->copy(*otherNode);

		addChild(thisNode);
	}
}

QStringList TreeNode::getChildList() const
{
	QStringList list;
	for(int i =0;i < m_children.size();i++) {
		TreeNode* node = m_children[i];
		assert(node != NULL);
		list += node->getName();
	}
	return list;
}

TreeNode::~TreeNode()
{
	for(int i = 0;i < m_children.size();i++) {
		TreeNode* node = m_children[i];
		delete node;
	}
}

void TreeNode::removeAll()
{
	for(int i = 0;i < m_children.size();i++) {
		TreeNode* node = m_children[i];
		delete node;
	}

	m_children.clear();
}

void TreeNode::dump(int parentCnt)
{
	QString text;
	text.sprintf("+- %s='%s'", stringToCStr(m_name),
			stringToCStr(m_data));

	for(int i = 0;i < parentCnt;i++)
		text  = "    " + text;

	debugMsg("%s", stringToCStr(text));
	for(int i = 0;i < (int)m_children.size();i++) {
		TreeNode *node = m_children[i];
		node->dump(parentCnt+1);
	}
}

void TreeNode::dump()
{
	dump(0);
}


Tree::Tree()
{
}

TreeNode *TreeNode::findChild(QString path) const
{
	QString childName;
	QString restPath;
	int indexPos;

	// Find the seperator in the string 
	indexPos = path.indexOf('/');
	if(indexPos == 0)
		return findChild(path.mid(1));

	// Get the first child name
	if(indexPos == -1)
		childName = path;
	else {
		childName = path.left(indexPos);
		restPath = path.mid(indexPos+1);
	}

	if(childName[0].toLatin1() == '#') {
		QString numStr = childName.mid(1);
		int idx = atoi(stringToCStr(numStr))-1;
		if(0 <= idx  && idx < getChildCount()) {
			TreeNode *child = getChild(idx);
			if(restPath.isEmpty())
				return child;
			else
				return child->findChild(restPath);
		}
	} else {
		// Look for the child
		for(int u = 0;u < getChildCount();u++) {
			TreeNode *child = getChild(u);

			if(child->getName() == childName) {
				if(restPath.isEmpty())
					return child;
				else
					return child->findChild(restPath);
			}
		}
	}

	return NULL;
}

QString Tree::getString(QString path) const
{
	TreeNode *node = m_root.findChild(path);
	if(node)
		return node->getData();
	return "";
}

int Tree::getInt(QString path) const
{
	QString str = getString(path);
	return str.toInt();
}

long long Tree::getLongLong(QString path) const
{
	QString str = getString(path);
	return stringToLongLong(stringToCStr(str));
}

int Tree::getChildCount(QString path) const
{
	int cnt = 0;
	TreeNode *node = m_root.findChild(path);
	if(node)
		cnt = node->getChildCount();

	return cnt;
}

QStringList Tree::getChildList(QString path) const
{
	QStringList list;
	TreeNode *node = m_root.findChild(path);
	if(node)
		list = node->getChildList();

	return list;
}

void Tree::removeAll()
{
	m_root.removeAll();
}

void Tree::copy(const Tree &other)
{
	m_root.copy(other.m_root);

}
