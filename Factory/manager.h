
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>



#define DFHLOG_W printf
#define DFHLOG_E printf
#define DFHLOG_I printf


class  AbstractMetaObjectBase {
 public:
  AbstractMetaObjectBase(const std::string &class_name,
                         const std::string &base_class_name,
                         const std::string &base_class_typeid)
      {}

  virtual ~AbstractMetaObjectBase() = default;

 private:
};

template <typename Base>
class AbstractMetaObject : public AbstractMetaObjectBase {
  static_assert(std::is_class<Base>::value, "Base must be a class");

 public:
  AbstractMetaObject(const std::string &class_name,
                     const std::string &base_class_name)
      : AbstractMetaObjectBase(class_name, base_class_name,
                               typeid(Base).name()) {}

  ~AbstractMetaObject() = default;

  AbstractMetaObject(const AbstractMetaObject &) = delete;
  AbstractMetaObject &operator=(const AbstractMetaObject &) = delete;

  virtual Base *Create() const = 0;
};

template <typename Derived, typename Base>
class MetaObject : public AbstractMetaObject<Base> {
  static_assert(std::is_base_of<Base, Derived>::value,
                "Derived must derive from Base");
 public:
  using AbstractMetaObject<Base>::AbstractMetaObject;
  MetaObject(const std::string &class_name,
                     const std::string &base_class_name)
                     : AbstractMetaObject<Base>(class_name,base_class_name){
    printf("class_name:%s,base_class_name:%s\n",class_name.c_str(), base_class_name.c_str());
    printf("Derived:%s,Base:%s\n",typeid(Derived).name(),typeid(Base).name());
  }

  Base *Create() const { return new Derived; }
};



using Path = std::string;
using ClassName = std::string;
using BaseClassTypeid = std::string;
using ObjectMap =
    std::unordered_map<ClassName, std::unique_ptr<AbstractMetaObjectBase>>;

class  LibraryManager {
 public:
  static LibraryManager *Instance(){
    if (instance_ == nullptr) {
            instance_ = new LibraryManager();
    }
    return instance_;
  };
  static int32_t RemoveInstance();

  template <typename Derived, typename Base>
  void RegisterPlugin(const std::string &class_name,
                      const std::string &base_class_name);

 private:
  LibraryManager() = default;
  ~LibraryManager() = default;

  template <typename Base>
  ObjectMap &GetObjectMapForBaseClass(){
    std::string base_class_typeid{typeid(Base).name()};
  if (all_meta_objects_.find(base_class_typeid) == all_meta_objects_.end()) {
    all_meta_objects_[base_class_typeid] = ObjectMap();
  }

  return all_meta_objects_[base_class_typeid];
  };

  std::vector<AbstractMetaObjectBase *> GetMetaObjectsForLibrary(
      const std::string &library_path);

 private:
  static LibraryManager *instance_;
  static std::mutex mutex_;

  std::mutex all_meta_objects_mutex_;
  std::unordered_map<BaseClassTypeid, ObjectMap> all_meta_objects_;
  std::mutex libraries_mutex_;
  std::string current_library_path_ = "";
  bool duplicate_class_ = false;
};
LibraryManager* LibraryManager::instance_ = new LibraryManager();

template <typename Derived, typename Base>
void LibraryManager::RegisterPlugin(const std::string &class_name,
                                    const std::string &base_class_name) {
                  

  std::lock_guard<std::mutex> lck{all_meta_objects_mutex_};
  ObjectMap &factory_map{GetObjectMapForBaseClass<Base>()};
  if (factory_map.find(class_name) != factory_map.end()) {
    DFHLOG_W(
        "A meta object exists with the same name for class {%s}, "
        "Program will exit\n",
        class_name.c_str());
    duplicate_class_ = true;
    return;
  }

  std::unique_ptr<AbstractMetaObjectBase> factory{
      new MetaObject<Derived, Base>(class_name, base_class_name)};
  if (factory == nullptr) {
    DFHLOG_E("Create meta object of class {%s} failed\n", class_name.c_str());
    return;
  }
  factory_map[class_name] = std::move(factory);
  DFHLOG_I("Register class {%s} success\n", class_name.c_str());
}

          

#define CLASS_LOADER_REGISTER_CLASS_INTERNAL(Derived, Base, UniqueID) \
  namespace {                                                         \
  struct ProxyClass##UniqueID {                                       \
    ProxyClass##UniqueID() {                                          \
    LibraryManager::Instance() \
    ->RegisterPlugin<Derived, Base>(#Derived, #Base); \
    printf("Derived:[%s],Base:[%s],UniqueID:[%d]\n",#Derived, #Base,UniqueID);\
    }                                                                 \
  };                                                                  \
  static ProxyClass##UniqueID g_register_class##UniqueID;             \
  }  // namespace

#define CLASS_LOADER_REGISTER_CLASS_INTERNAL_1(Derived, Base, UniqueID) \
  CLASS_LOADER_REGISTER_CLASS_INTERNAL(Derived, Base, UniqueID)

#define CLASS_LOADER_REGISTER_CLASS(Derived, Base) \
  CLASS_LOADER_REGISTER_CLASS_INTERNAL_1(Derived, Base, __COUNTER__)






class Module
{
private:
    /* data */
public:
    Module(/* args */){};
    ~Module(){};
    virtual void init() = 0;
};

class ModuleFactoryBase {
 public:
  ModuleFactoryBase() = default;
  virtual ~ModuleFactoryBase() = default;

  virtual std::shared_ptr<Module> CreateModule() = 0;
};

template <typename MDerived>
class ModuleFactory : public ModuleFactoryBase {
  static_assert(std::is_base_of<Module, MDerived>::value,
                "MDerived must derive from Module");

 public:
  ModuleFactory() = default;
  ~ModuleFactory() override = default;

  std::shared_ptr<Module> CreateModule() override {
    return std::make_shared<MDerived>();
  }
};

#define REGISTER_MODULE(DerivedClass)                      \
  CLASS_LOADER_REGISTER_CLASS(ModuleFactory<DerivedClass>, \
                              ModuleFactoryBase)



