/********************************************************************
	created:	2013/07/01
	created:	1:7:2013   16:31
	filename: 	i:\AB3.0\3H_FST\FST\common\singleton.h
	file path:	i:\AB3.0\3H_FST\FST\common
	file base:	singleton
	file ext:	h
	author:		ybb
	
	purpose:	单例实现(参考BOOST)
*********************************************************************/
#ifndef __FST_SINGLETON_INCLUDE__
#define __FST_SINGLETON_INCLUDE__

namespace pattern
	{

template<typename T>
struct singleton
{
	struct object_creator
	{
	object_creator(){singleton<T>::instance();}
	inline void do_nothing() const {}
	} ; 	
static object_creator  the_create_object ; 

public:
	typedef T object_type ; 
	
	static object_type& instance()
		{
		static object_type obj; 
		the_create_object.do_nothing() ; 
		return obj ;
		}
	} ; 

template<typename T>
typename singleton<T>::object_creator singleton<T>::the_create_object ; 

	} ; 

#define DECLARE_SINGLETON_CLASS(T)  friend struct pattern::singleton<T> ; 


#endif 