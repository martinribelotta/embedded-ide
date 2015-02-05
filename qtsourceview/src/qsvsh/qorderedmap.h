#ifndef __QE_ORDERED_MAP_H__
#define __QE_ORDERED_MAP_H__

#include <QString>
#include <QList>
#include <QTextCharFormat>

template <class Key, class Value>
class QOrderedMapNode
{
public:
	Key	key;
	Value	value;
	QOrderedMapNode()
	{
	}

	QOrderedMapNode( Key k, Value v )
	{
		key  = k;
		value = v;
	}
};


/**
 * \class QOrderedMap
 * \brief Another hash table implementation
 * 
 * A string hash table. The twist, is that the objects are ordered inside
 * at the same order they were inserted (unlike QMap which orders by key or 
 * QHash which orders by arbitrarily order).
 * 
 * \see QHash
 * \see QMap
 */
template <class Key, class Value>
class QOrderedMap
{
public:
	QOrderedMap();
	~QOrderedMap();
	
	void add( Key k, Value v );
	void remove( Key k );
	
	void clear();
	bool contains( Key key );
	int count();
	bool empty();
	QList< QOrderedMapNode< Key, Value > > & keys();
	Value &operator[]( Key k );
	Value &value( Key k );
	
private:
	QList< QOrderedMapNode< Key, Value > > nodes;
};


// implementation

/**
 * \brief default construstor
 * 
 * Creates and initializes the ordered map object.
 */
template <class Key, class Value>
QOrderedMap<Key, Value>::QOrderedMap()
{
	// construst the object... yey!!!
}

/**
 * \brief default destructor
 * 
 * Destruct the ordered map object. Will clear the internal
 * QList object.
 */
template <class Key, class Value>
QOrderedMap<Key, Value>::~QOrderedMap()
{
	// destroy the object... yey!!!
	nodes.clear();
}

/**
 * \brief Inserts a new item with the key key and a value of value.
 * \param k the new key
 * \param v the value of key \b k
 *
 * Inserts a new item with the key key and a value of value.
 * If there is already an item with the key key, that item's value 
 * is replaced with value.
 *
 * If there are multiple items with the key key, the most recently 
 * inserted item's value is replaced with value.
 */
template <class Key, class Value>
void QOrderedMap<Key, Value>::add( Key k, Value v )
{
	if (contains(k))
		remove( k );
		
	QOrderedMapNode<Key, Value> node( k, v );
 	nodes.append( node );
}

/**
 * \brief Removes all the items that have the key key from the hash.
 * \param k the keys to be removed
 * 
 * Removes the item that have the key key from the hash. 
 * 
 */
template <class Key, class Value>
void QOrderedMap<Key, Value>::remove( Key k )
{
	int i = 0;
	QOrderedMapNode<Key, Value> n;
	foreach( n, nodes )
	{
		i ++;
		if (n.key == k )
		{
			//nodes.erase( n );
			nodes.removeAt( i );
			return;
		}
	}
}

/**
 * \brief Clears the map
 * 
 * Clears the contents of the map. No keys/values
 * will be fonud on the map after calling this function.
 * 
 */
template <class Key, class Value>
void QOrderedMap<Key, Value>::clear()
{
	nodes.clear();
}

/**
 * \brief Check for the in the map
 * \param key the key to be tested
 * \return true if the key is found on the map, false otherwise
 * 
 * This function checks if the key is found in the ordered map.
 * 
 */
template <class Key, class Value>
bool QOrderedMap<Key, Value>::contains( Key key )
{
	QOrderedMapNode<Key, Value> n;
	foreach( n, nodes )
	{
		if (n.key == key )
			return true;
	}
	
	return false;
}

/**
 * \brief Returns the ammonut of keys on this map
 * \return The number of keys on this map
 * 
 * This function returns the number of keys on the map.
 */
template <class Key, class Value>
int QOrderedMap<Key, Value>::count()
{
	return nodes.count();
}

/**
 * \brief Check if the map is empty
 * \return true if the map is empty
 * 
 * Returns true if there are no keys on the map.
 */
template <class Key, class Value>
bool QOrderedMap<Key, Value>::empty()
{
 	return nodes.empty();
}

/**
 * \brief Return the internal representation of the keys
 * \return The internally used keys
 * 
 * If for some reason you need to access the list of keys 
 * and modify them directly, you can use this function to get
 * a copy of the keys. 
 * 
 * Modifications to the list will not seen will not be propagated
 * to this map. The returned value is not a reference.
 * 
 */
template <class Key, class Value>
QList< QOrderedMapNode< Key, Value > >& QOrderedMap<Key, Value>::keys()
{
	return nodes;
}

/**
 * \brief Return the value for key
 * \param k The key you are looking for
 * 
 * This is an overloaded member function, provided for convenience.
 * It behaves exactly like value( Key )
 * 
 * \see value ( Key )
 */
template <class Key, class Value>
Value& QOrderedMap<Key, Value>::operator[](Key k)
{
	return value( k );
}

/**
 * \brief Get the value of a key
 * \param k The key to search for
 * \return The value of the key, nothing if not found
 * 
 * This function returns the value of the key you are looking.
 * If the key is not found, it will nor return any value at all,
 * and it's a wise idea to check if the key is in the map
 * before calling this function.
 * 
 * \see contains( Key )
 */
template <class Key, class Value>
Value& QOrderedMap<Key, Value>::value( Key k )
{
	QOrderedMapNode<Key, Value> n;
	foreach( n, nodes )
	{
		if (n.key == k )
			return n.value;
	}
}

#endif // __QE_ORDERED_MAP_H__
