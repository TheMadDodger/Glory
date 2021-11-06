#pragma once
#include "UUID.h"

/* *IDEA FOR DETECTING OBJECT INHERITENCE AND OBJECT TYPE*
* The Object class keeps track of an array of all the inherited classes,
* the inherited class could add itself to this aray in its constructor,
* could be made easier with the use of a macro that generates the constructor for you,
* this would mean that at index 0 of the array you have the last class of inheritence,
* while the last index is the base class at the top of the hierarchy.
* Now when searching for a specific type you check for index 0,
* and when searching if an object is inheriting a specific class
* you check if that type is present in the array.
*/


namespace Glory
{
	class Object
	{
	public:
		Object();
		Object(UUID uuid);
		virtual ~Object();

		const UUID& GetUUID() const;

	private:
		UUID m_ID;
	};
}