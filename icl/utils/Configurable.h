// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/Exception.h>
#include <icl/utils/AutoParse.h>
#include <icl/utils/UncopiedInstance.h>
#include <icl/utils/prop/Adapter.h>

#include <any>
#include <functional>
#include <typeindex>

#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <mutex>

namespace icl::utils {
  /// Interface for classes that can be configured from configuration-files and GUI-Components
  /** The Configurable-interface can be used to define a classes
      parameters/properties that shall be changed at runtime. The
      Configurable-subclasses can define properties that can be
      accessed by string identifiers. Each property has a type, a
      type-dependend description of possible values, a current value
      and a so called volatileness. Please see class interface and
      it's function descriptions for more details. A list of
      supported property types is provided in the documentation of
      the method icl::Configurable::getPropertyType

      \section IMPL Implementing the Configurable Interface

      It is strongly recommended to use the Configurable's property
      storage mechanism to manage a classes properties. Special
      behaviour to the adaption of certain properties can easily be
      added by registering a callback to an own member
      function. Alternatively, all Configurable's virtual methods
      can be reimplemented to obtain special behaviour. In this case
      the programmer himself can provide storage for the classes
      properties, but this is -- as said above -- not recommended
      due to the complex interface.


      \section CP Child Configurables

      Configurables can not only have a list of properties that can
      be got and set, but also a list of chlidren. All
      child-properties will also become properties of it's
      parent. However, the first section prefix (which is used for
      the property-tab's label) can be adapted. <b>Note</b> that
      this behaviour must be preserved if the virtual functions
      setPropertyValue and getPropertyValue are
      reimplemented. Usually, you can simply call
      Configurable::[set/get]PropertyValue(...) at the end of you
      versions of these methods.

      \section REG Configurable Registration

      Configurable class should be registered statically using one
      of the two registration macros REGISTER_CONFIGURABLE or
      REGISTER_CONFIGURABLE_DEFAULT. This is strongly recommended
      since the class interface of a configurable class does not
      give information about the properties that are provided by a
      specific Configurable class. Instead, all classes, that
      implement the Configurable interface, can be registered
      statically, which allows for runtime exploration of possible
      Configurable classes and their supported properties.

      The example application <b>icl-configurable-info</b> can be
      used to explore allowed properties.


      In order to make the static registration process as easy as
      possible, special macros are provided. Example:

      \code
      namespace icl{
        // MyConfigurable.h
        struct MyConfigurable{
           MyConfigurable();
           void foo(){}
           ...
        };
      }

      // MyConfigurable.cpp
      namespace icl{
        MyConfigurable::MyConfigurable(){
          addProperty(....);
        }
        void MyConfigurable::foo() {...}

        // registration at the end of the .cpp file
        // within the icl-namespace
        REGISTER_CONFIGURABLE_DEFAULT(MyConfigurable);
      }
      \endcode

      If no default constructor is available, the macro
      REGISTER_CONFIGURABLE can be used. Here, you can also specify
      how an instance of that class is created. Example:

      \code
      namespace icl{
        // MyComplexConfigurable.h
        struct MyComplexConfigurable{
           // no default constructor
           MyComplexConfigurable(int i, float j);
           void foo(){}
           ...
        };
      }

      // MyComplexConfigurable.cpp
      namespace icl{
        MyComplexConfigurable::MyComplexConfigurable(int i, float j){
          addProperty(....);
        }
        void MyComplexConfigurable::foo() {...}

        // provide default arguments here
        REGISTER_CONFIGURABLE(MyComplexConfigurable, return new MyComplexConfigurable(1,4.5));
      }
      \endcode

      For classes with pure-virtual methods, it is recommended to
      provide a dummy non-virtual extension of that class whose name
      is extended by a _VIRTUAL postfix. In this case, listing the
      Configurable classnames shows explicitly, that a class is a
      virtual interface.  Example:

      \code
      namespace icl{
        // MyVirtualConfigurable.h
        struct MyVirtualConfigurable{
           MyVirtualConfigurable();
           // pure virtual method
           virtual void foo(int bar) = 0;
           ...
        };
      }

      // MyVirtualConfigurable.cpp
      namespace icl{
        MyVirtualConfigurable::MyVirtualConfigurable(){
          addProperty(....);
        }

        struct MyVirtualConfigurable_VIRTUAL : public MyVirtualConfigurable{
          virtual void foo(int){}
        };

        // register the dummy implementation
        REGISTER_CONFIGURABLE_DEFAULT(MyVirtualConfigurable_VIRTUAL);
      }
      \endcode

      \section EX Examples

      There are several examples available in the ICL-source
      try. Use the ICL-tool <b>icl-configurable-info -list</b> to
      obtain a list of all Configurable implementations and their
      supported properties.
  */
  class Configurable;  // fwd for the refs below

  /// Proxy over a property's value.  Created short-term by
  /// `Configurable::prop(name).value`.  All operations route through
  /// `setPropertyValueTyped` / `getPropertyValue`, keeping invariants
  /// (typed_value up to date, callbacks fired) automatically.
  ///
  /// Holds a pointer to the owning Configurable and a pointer to the
  /// name string.  Safe for the lifetime of the `PropertyHandle` the
  /// ref was constructed from.
  class PropertyValueRef {
    Configurable      *m_conf;
    const std::string *m_name;
   public:
    PropertyValueRef(Configurable *c, const std::string &n) : m_conf(c), m_name(&n) {}

    /// Typed write — stores std::any(v) via setPropertyValueTyped,
    /// no stringify/parse round-trip.  Accepts any T.
    template<class T>
    PropertyValueRef& operator=(T&& v);  // defined after Configurable

    /// Explicit const char* overload — the templated one would deduce
    /// T = const char* and store a pointer into std::any.  Forwards
    /// to the std::string path so downstream readers cast cleanly.
    PropertyValueRef& operator=(const char* s);

    /// Templated implicit read — AutoParse<std::any> cascade over
    /// typed_value: exact any_cast fast path, numeric widening, parse
    /// of stored string, stringify-numeric on T=std::string.
    template<class T>
    operator T() const;  // defined after Configurable

    /// Explicit read.
    template<class T>
    T as() const { return static_cast<T>(*this); }

    /// Stringify the stored value.  Mirrors `AutoParse<std::any>::str()`
    /// — equivalent to `as<std::string>()` but named for parity with
    /// the pre-proxy `getPropertyValue(name).str()` idiom.  Throws the
    /// same exception as `as<std::string>()` on non-stringifiable
    /// property kinds (e.g. `prop::Command`).
    std::string str() const { return static_cast<std::string>(*this); }

    /// Equality sugar for the common `prop("x").value == "on"` idiom.
    bool operator==(std::string_view s) const;
    bool operator==(const char* s) const;
    template<class T>
    bool operator==(const T& t) const { return static_cast<T>(*this) == t; }
  };

  class ICLUtils_API Configurable{

    friend class ConfigurableProxy;

    public:
    /// Represents a single property
    struct Property{
      Property():configurable(0),volatileness(0){}
      Property(Configurable *parent,
               const std::string &name,
               int volatileness, const std::string &tooltip)
        : configurable(parent), name(name),
          volatileness(volatileness), tooltip(tooltip){}
      Configurable *configurable; //!< corresponding Configurable
      std::string name;  //!< property-ID
      int volatileness;  //!< volatileness of a this property (0= no-volatileness, X=expected update every X msec)
      std::string tooltip; //!< property description, that is also used as tooltip
      std::string childPrefix;
      /// Structured constraint payload (`prop::Range<T>`, `prop::Menu<T>`,
      /// `prop::Flag`, ...).  Populated by the typed `addProperty<C>`
      /// overload; empty for properties that were registered via the legacy
      /// string-taking `addProperty(name, "type", "info", value)` path.
      ///
      /// Once all call sites migrate to the typed overload, qt::Prop will
      /// drive its widget dispatch off this field (via
      /// `prop::lookupAdapter` / constraint-type introspection) instead of
      /// parsing the `type` + `info` strings.  During the transition qt::Prop
      /// falls back to the string path when `constraint` is empty.
      std::any constraint;
      /// Typed current value, stored as `std::any` of the declared C++
      /// type (e.g. `float` for `prop::Range<float>`, `std::string` for
      /// `prop::Menu<std::string>`).  Populated by:
      ///   (1) the typed `addProperty<C>` overload at registration time;
      ///   (2) `setPropertyValue` when the property has a known `constraint`
      ///       (the adapter's `fromString` parses the incoming string into
      ///       the declared type).
      /// `getPropertyValue` prefers this field over the legacy `value`
      /// string: `AutoParse<std::any>` wraps it directly, giving callers
      /// a fast path with no string round-trip.  When empty, `value`
      /// (still populated by every setter) is wrapped as
      /// `std::any(std::string)` instead.
      std::any typed_value;

      /// Typed read of `typed_value` through `AutoParse<std::any>`'s
      /// cascade: exact `any_cast<T>` fast path, numeric widening for
      /// arithmetic `T`, `parse<T>` if stored as string, stringify on
      /// `T = std::string` request.
      ///
      /// Preferred over `p.as<T>()` in callback bodies — zero
      /// string round-trip in the common case where the stored type
      /// matches the requested T (which it does for every typed- or
      /// legacy-dispatched property after step 9 commit 1).
      template<class T>
      T as() const {
        return AutoParse<std::any>(typed_value).template as<T>();
      }

      /// for more efficient find
      bool operator==(const std::string &name) const { return this->name == name; }
    };

    private:

    /// by default internally use property list
    using PropertyMap = std::map<std::string,Property, std::less<>>;

    /// list of all properties
    PropertyMap m_properties;

    /// whether to use property ordering
    bool m_isOrdered;
    /// ordering of the properties
    std::map<int,std::string> m_ordering;

    /// internal list of child configurables
    std::map<const Configurable*, std::string> m_childConfigurables;

    /// internal pointer to elder configurable
    Configurable* m_elderConfigurable;

    /// internal ID, that is used to provide global access to all instantiated configurables at runtime by given ID;
    std::string m_ID;

    /// static list of all instantiated Configurables
    static std::map<std::string,Configurable*, std::less<>> m_instances;

    /// list of patterns for deactiavted properties
    std::vector<std::string> m_deactivated;

    /// locks all accesses to property values
    /** adding and adapting properties is not thread safe! */
    mutable UncopiedInstance<std::recursive_mutex> m_mutex;

    protected:

    /// This can be used by derived classes to store supported properties in the internal list
    /** Throws an exception if the property name is already defined.
        Note: properties names can structured using '.'-delimiters.
        e.g. properties like general.threshold,general.mean,filer.mask-size.
        This is then later translated into a structured configuratable GUI.
        */
    void addProperty(const std::string &name, const std::string &type,
                     const std::string &info,
                     const AutoParse<std::string> &value=AutoParse<std::string>(),
                     const int volatileness=0, const std::string &tooltip=std::string());

    /// Typed addProperty — registers a property using a structured
    /// `prop::Range<T>` / `prop::Menu<T>` / `prop::Flag` / ... constraint
    /// instead of the legacy `(type, info)` string pair.
    ///
    /// Call sites read uniformly:
    /// \code
    ///   addProperty("gain",    prop::Range{.min=0.f, .max=500.f},        250.f);
    ///   addProperty("gamma",   prop::Range{.min=0, .max=100, .ui=UI::Spinbox}, 50);
    ///   addProperty("mode",    prop::Menu{"fast", "slow", "auto"},       "fast");
    ///   addProperty("enabled", prop::Flag{},                             true);
    ///   addProperty("save",    prop::Command{});
    /// \endcode
    ///
    /// Under the hood this looks up the constraint's adapter via
    /// `prop::lookupAdapter` and calls the legacy string-based overload
    /// with synthesized `type` / `info` / `value` strings, then stores the
    /// structured constraint on the resulting `Property` so downstream
    /// code (qt::Prop) can dispatch off the typed constraint.
    ///
    /// Until qt::Prop flips to constraint-driven dispatch (step 5 of the
    /// migration), both the stringly-typed legacy data and the `std::any`
    /// constraint payload are maintained — no behavioural change.
    template<class C>
    void addProperty(const std::string &name,
                     C constraint,
                     typename C::value_type initial = {},
                     const int volatileness = 0,
                     const std::string &tooltip = std::string()) {
      const auto &a = prop::lookupAdapter(std::type_index(typeid(C)));
      std::any constraintAny(std::move(constraint));
      std::any valueAny(std::move(initial));
      const std::string type  = a.typeId(constraintAny);
      const std::string info  = a.infoString(constraintAny);
      const std::string value = a.toString(valueAny);

      // Register through the string path — this populates the legacy
      // type/info/value on Property and flows through the existing
      // ordered-properties / child-configurable / duplicate-name logic.
      addProperty(name, type, info, AutoParse<std::string>(value),
                  volatileness, tooltip);

      // Attach the structured payload + typed value.  prop_storage
      // returns the underlying Property& for direct-field access;
      // external callers use prop(name) which returns the handle
      // proxy instead.
      auto &p = prop_storage(name);
      p.constraint  = std::move(constraintAny);
      p.typed_value = std::move(valueAny);
    }

    /// This adds another configurable as child
    /** Child configurables can be added with a given prefix. If this prefix is not "",
        the childs properties will get an own tab in the configurables GUI.
        <b>Note: if the prefix is not "", it should end with a '.' character. If not,
        an additional '.' is added automatically in order to move all properties
        into a dedicated tab</b> */
    void addChildConfigurable(Configurable *configurable, const std::string &childPrefix="");

    /// removes the given child configurable
    void removeChildConfigurable(Configurable *configurable);

    public:

    /// Structured handle for a property — value proxy + field refs.
    /// Returned from `prop(name)` below.  Defined after Configurable so
    /// it can delegate to Configurable methods.
    class Handle;

    /// Look up a property by name and return a short-lived handle with
    /// a write-through value proxy + reference members to the Property
    /// fields (name, tooltip, constraint, typed_value, volatileness,
    /// childPrefix) and computed accessors (type(), info()).
    ///
    /// Public so both subclass-internal code (`prop("gain").value = 0.5f`)
    /// and external callers (`conf->prop("x").constraint`, apps, tests)
    /// share one entry point — the Handle is the designed API surface,
    /// not an internal subclass convenience.  The public
    /// `getPropertyValue` / `setPropertyValue` / `getPropertyConstraint`
    /// / `getPropertyToolTip` / `getPropertyVolatileness` stay as thin
    /// wrappers for back-compat.
    Handle prop(const std::string &propertyName);

    /// Const overload — writes through PropertyValueRef are still
    /// possible via internal const_cast; this matches the existing
    /// semantic where const-ness of the Configurable doesn't gate
    /// property writes (setPropertyValue is public and non-const on
    /// non-const `this`, but external writers routinely const_cast or
    /// hold by non-const pointer).  A separate pass could split out a
    /// read-only ConstHandle if tightening const-correctness becomes
    /// worth the API churn.
    Handle prop(const std::string &propertyName) const;

    protected:

    private:
    /// Internal Property& accessor — used by Configurable's own .cpp
    /// setters/getters and by the inline addProperty<C> template to
    /// poke the typed_value / constraint fields directly.  Subclasses
    /// should not touch Property's fields directly; they go through
    /// prop(name).* or set/getPropertyValue{,Typed}.
    /// Throws `ICLException` if the property is not registered.
    Property &prop_storage(const std::string &propertyName);
    const Property &prop_storage(const std::string &propertyName) const;

    protected:

    /// create this configurable with given ID
    /** all instantiated configurables are globally accessible by static getter functions
        If given ID is "", then this configurable is not added to the static list.
        Configurables can later be put into the static list by using setConfigurableID
        **/
    Configurable(const std::string &ID="", bool ordered=true);

    public:

    /// virtual destructor
    virtual ~Configurable(){}

    /// Copy constructor
    /** the configurable ID is not copied. Use setConfigurableID afterwards */
    Configurable(const Configurable &other);

    /// Assignment operator
    /** the configurable ID is not copied. Use setConfigurableID afterwards */
    Configurable &operator=(const Configurable &other);

    /// sets the ID of this configurable
    /** The ID is used for accessing the configurable globally*/
    void setConfigurableID(const std::string &ID);

    /// returns the configurables static ID
    inline const std::string &getConfigurableID() const{
      return m_ID;
    }

    /// returns whether the ordered flag is set
    bool isOrderedFlagSet() const{
      return m_isOrdered;
    }

    /// adds an additional deativation pattern
    /** By this means, extended or also accumulating Configurable-instances can
        hide some of their inherited/accumulated properties. E.g. special
        ICLFilter::UnaryOp instances can hide the UnaryOp-properties 'Clip To ROI'
        and 'Check Only' if it is not appropriate to adapt these.
        The given pattern can be an abitrary regular expression which
        compared icl::match(const std::string&, const std::string&,int);
        E.g. to deactivate all UnaryOp-properties, you can add the filter reg-ex
        '^UnaryOp' more precisesly, you could also add the filter-reg-ex
        '^UnaryOp\..*' which ensures that a '.' and some other characters follow
        the 'UnaryOp' string.
        Commonly the '^Foo' can be used to remove all properties with given prefix 'Foo'.
        */
    void deactivateProperty(const std::string &pattern);

    /// removed a formerly added deactivation pattern
    /** Please note, that usually this should not be used because there is
        most probably a reason why a property was deactivated before. */
    void deleteDeactivationPattern(const std::string &pattern);

    /// this returns a filtered list of properties (using all filters added by deactivateProperty)
    std::vector<std::string> getPropertyListWithoutDeactivated() const;

    /// this function can be used to adapt a specific property afterwards
    /** <b>Please note:</b> The newly given type and info value must of cause be compatible with the current
        property type. The type of a property cannot be changed. <b>Please note also, that this
        Function also needs to be reimplemented if getPropertyInfo or getPropertyType was reimplemented.</b>
        A range property can e.g. be adapted to a menu property, which then again restricts possible values.
        You can also set a properties type from range:spinbox to range:slider
        */
    void adaptProperty(const std::string &name, const std::string &newType,
                       const std::string &newInfo, const std::string &newToolTip);


    /// this function can be used in subclasses to create a default ID
    /** The ID will be the first not used prefixNUMBER where number is
        an integer value 0,1,... */
    static std::string create_default_ID(const std::string &prefix);


    /// Function type for changed properties
    using Callback = std::function<void(const Property&)>;

    /// add a callback for changed properties
    void registerCallback(const Callback &cb){
      callbacks.push_back(cb);
    }

    /// removes a callback that was registered before
    void removedCallback(const Callback &cb);

    /// this can be used to let this instance also apply property changes to others
    /** Please take care to not create cyclic dependency graphs */
    void syncChangesTo(Configurable *others, int num=1);

    protected:
    /// internally managed list of callbacks
    std::vector<Callback> callbacks;

    /// calls all registered callbacks
    /**
    @param propertyName the name of the property to check
    @param caller the instance calling the function
    **/
    void call_callbacks(const std::string &propertyName,const Configurable* caller) const;

    public:

    /// used as shortcut -- just an empty vector of std::strings
    static const std::vector<std::string> EMPTY_VEC;

    /// returns configurable by given ID
    /** returns instantiated Configurable by given ID or NULL, if the ID was not found */
    static Configurable* get(const std::string &id);


    /// sets a property value
    /** If the property is actually owned by a child-configurable, the
        function forwards to that configurable.  Non-virtual: property
        side-effects belong in a registerCallback() lambda, not in a
        subclass override of this function.  ConfigFile save/load and
        syncChangesTo callers use this entry point. */
    void setPropertyValue(const std::string &propertyName, const AutoParse<std::string> &value);

    /// Typed setter — stores `v` directly into Property::typed_value,
    /// fires callbacks.  Bypasses the string round-trip that
    /// `setPropertyValue(name, AutoParse<std::string>)` pays (stringify
    /// caller's T → parse via adapter back to declared type).
    ///
    /// Prefer this over the string setter when you already have the
    /// typed value in hand:
    /// ```
    ///   c.setPropertyValueTyped("gain", std::any(0.5f));    // direct
    ///   c.setPropertyValue("gain", str(0.5f));              // string round-trip
    /// ```
    ///
    /// For properties with a known `constraint`, the stored any is
    /// expected to match the constraint's value_type.  Downstream
    /// readers (AutoParse<std::any>) will any_cast<T> off typed_value
    /// directly in that case — zero parse.  For unconstrained
    /// properties (rare edge case), any std::any is accepted and
    /// downstream reads go through the cascade.
    void setPropertyValueTyped(const std::string &propertyName, std::any v);

    /// Silent typed write — updates `Property::typed_value` but does
    /// NOT fire callbacks.
    ///
    /// Intended for volatile `prop::Info` or similar display-only
    /// properties that an owner updates from a latency-sensitive path
    /// (e.g. a Grabber's acquireImage() under `m_grabMutex`): the
    /// qt::Prop GUI already polls such properties via its volatileness
    /// timer, so firing callbacks on every update is redundant and can
    /// produce lock inversions when a callback reaches for a lock that
    /// the writer's context already depends on (seen in practice with
    /// DemoGrabber updating "current-pos" under m_grabMutex while the
    /// qt::Prop callback wants execMutex, and a GUI slider drag
    /// holding execMutex wants m_grabMutex through the Grabber-wrapped
    /// property-change callback).
    ///
    /// Semantically equivalent to `setPropertyValueTyped` minus the
    /// `call_callbacks()` tail — all other consumers of the property
    /// (getPropertyValue, Handle::value, ConfigFile save) see the new
    /// value immediately.  The GUI polls via VolatileUpdater and
    /// notices the change on its next tick.
    void setPropertyValueSilent(const std::string &propertyName, std::any v);

    // setPropertyPayload / getPropertyPayload retired — typed_value itself
    // carries the image/non-string payload now that addProperty<ImageView>
    // stores live core::Image directly.  Use
    //   prop(name).value = img;         // writer
    //   auto img = prop(name).typed_value  // reader (std::any holding Image)
    // instead.

    /// returns a list of All properties, that can be set using setProperty
    /** This function should usually not be used. Instead, you should call getPropertyListWithoutDeactivated
        @return list of supported property names **/
    std::vector<std::string> getPropertyList() const;

    /// base implementation for property check (seaches in the property list)
    bool supportsProperty(const std::string &propertyName) const;

    /// writes all available properties into a file
    /** @param filename destination xml-filename
        @param propertiesToSkip some common grabber parameters e.g. trigger-settings cause problems when
        they are read from configuration files, hence these parameters are skipped at default*/
    void saveProperties(const std::string &filename, const std::vector<std::string> &propertiesToSkip=EMPTY_VEC) const;

    /// reads a camera config file from disc
    /** @ see saveProperties */
    void loadProperties(const std::string &filename,const std::vector<std::string> &propertiesToSkip=EMPTY_VEC);

    /// get type of property
    /** This is a new minimal configuration interface: When implementing generic
        video device configuration utilities, the programmer needs information about
        the properties received by getPropertyList(). With the getType(const string&)
        function, you can explore
        all possible params and properties, and receive a type string which defines
        of which type the given property was: \n
        (for detailed description of the types, see also the get Info function)
        Types are:
        - "range" this is the same as "range:slider" (see below)
        - "range:slider" the property is a double value in a given range
        - "range:spinbox" the property is an int-value in given range with stepping 1
        - "value-list" the property is a double value in a list of possible values
        - "menu" the property is a string value in a list of possible values
        - "flag" the property is a boolean flag that can be set to "on" and "off"
        - "command" property param has no additional parameters (this feature is
        - "float" the property value can be any valid float value within a given range
        - "int" the property value can be any valid int value within a given range
        - "string" the property is a string value with a given maximum length
        - "color" the property has RGBA values in range 0-255
        used e.g. for triggered abilities of grabbing devices, like
        "save user settings" for the PWCGrabber
        - "info" the property is an unchangable internal value (it cannot be set actively)
        - ... (propably some other types are defined later on)
        */
    std::string getPropertyType(const std::string &propertyName) const{
      const Property &p = prop_storage(propertyName);
      if(!p.constraint.has_value()) return "";
      return prop::lookupAdapter(p.constraint.type()).typeId(p.constraint);
    }

    /// get information of a properties valid values
    /** This is the second function of the minimal configuration interface: If
        received a specific property type with getType(), it's
        possible to get the corresponding range, value-list or menu with this
        funcitons. The Syntax of the returned strings are:
        - "[A,B]:C"  for a range with min=A, max=B and stepping = C or [A,B] with no stepping
        - "[A,B]" for float- and int-properties with min=A and max=B
        - ",A,B,C,..." for a value-list and A,B,C are ascii doubles (real commas can be escaped using \)
        - ",A,B,C,..." for a menu and A,B,C are strings (real commas can be escaped using \)
        - nothing for "info"- and "color"-typed properties
        - MAX_LENGTH for string typed properties
        - flag-properties always have the possible values "on|1|true" or "off|0|false"
        <b>Note:</b> The received string can be translated into C++ data
        with some static utility function in this Grabber class.
        */
    std::string getPropertyInfo(const std::string &propertyName) const{
      const Property &p = prop_storage(propertyName);
      if(!p.constraint.has_value()) return "";
      return prop::lookupAdapter(p.constraint.type()).infoString(p.constraint);
    }

    /// returns the current value of a property or a parameter
    /** If the property is actually owned by a child-configurable,
        the function forwards to that configurable.
        Returns an `AutoParse<std::any>` backed either by the typed
        value (when set via `addProperty<C>` or when `setPropertyValue`
        could parse into the declared type) or — as a fallback — by the
        legacy string value wrapped as `std::any(std::string)`.
        Callers writing `T x = c.getPropertyValue(name)` continue to
        work via `AutoParse<std::any>`'s extraction cascade (exact cast
        → numeric widen → parse-if-string → stringify-if-numeric).
        Callers that specifically need a string can use
        `.str()` / `.as<std::string>()`. */
    AutoParse<std::any> getPropertyValue(const std::string &propertyName) const;

    // getPropertyConstraint / getPropertyToolTip / getPropertyVolatileness
    // retired — the Handle returned from prop(name) exposes them as const
    // references to Property::{constraint, tooltip, volatileness} (no copy,
    // no virtual dispatch).

    /// registers a configurable type
    /** @see \ref REG */
    static void register_configurable_type(const std::string &classname, std::function<Configurable*()> creator);

    /// returns a list of all registered configurable classnames
    /** @see \ref REG */
    static std::vector<std::string> get_registered_configurables();

    /// creates a configurable by given name
    /** @see \ref REG */
    static Configurable *create_configurable(const std::string &classname);
  };

  /// Short-lived handle returned from `Configurable::prop(name)`.
  ///
  /// Bundles:
  ///   - a `PropertyValueRef value` — write-through proxy
  ///   - reference members for the Property's const-readable fields
  ///     (name, type, info, tooltip, childPrefix, constraint,
  ///      typed_value, volatileness)
  ///   - a templated `.as<T>()` for typed reads
  ///
  /// Intentionally non-assignable (reference members); valid only for
  /// the expression in which prop() was called.  Don't bind an
  /// `auto &h = c.prop("x")` across statements — the handle's refs
  /// stay pointing at the stored Property, but the idiom is for
  /// line-local use.
  class Configurable::Handle {
    Configurable *m_conf;
    Property     *m_p;
   public:
    Handle(Configurable *c, Property &p)
      : m_conf(c), m_p(&p),
        value{c, p.name},
        name(p.name),
        tooltip(p.tooltip), childPrefix(p.childPrefix),
        constraint(p.constraint), typed_value(p.typed_value),
        volatileness(p.volatileness) {}

    Handle(const Handle&) = default;
    Handle& operator=(const Handle&) = delete;  // refs can't rebind

    /// Write-through value proxy — `h.value = v` calls
    /// `Configurable::setPropertyValueTyped(name, std::any(v))`.
    PropertyValueRef value;

    /// Reference members — read access to Property fields with
    /// identical syntax to the pre-step-9 `Property&` era.
    const std::string &name;
    const std::string &tooltip;
    const std::string &childPrefix;
    const std::any    &constraint;
    const std::any    &typed_value;
    const int         &volatileness;

    /// Legacy type / info strings — synthesized on demand via the
    /// constraint adapter.  `type` / `info` are no longer stored on
    /// Property (retired step 8 of the Session 53 arc), so these
    /// return by value rather than as reference members.  Equivalent
    /// to `configurable->getPropertyType(name)` / `...Info(name)` but
    /// reachable through the handle for one-stop introspection.
    std::string type() const;
    std::string info() const;

    /// Typed read — forwards to `Property::as<T>`.
    template<class T>
    T as() const { return m_p->template as<T>(); }
  };

  // PropertyValueRef inline definitions — need Configurable's complete
  // type (setPropertyValueTyped / getPropertyValue) visible.
  template<class T>
  PropertyValueRef& PropertyValueRef::operator=(T&& v) {
    m_conf->setPropertyValueTyped(*m_name, std::any(std::forward<T>(v)));
    return *this;
  }
  inline PropertyValueRef& PropertyValueRef::operator=(const char* s) {
    return *this = std::string(s);
  }
  template<class T>
  PropertyValueRef::operator T() const {
    return m_conf->getPropertyValue(*m_name).template as<T>();
  }
  inline bool PropertyValueRef::operator==(std::string_view s) const {
    return static_cast<std::string>(*this) == s;
  }
  inline bool PropertyValueRef::operator==(const char* s) const {
    return static_cast<std::string>(*this) == s;
  }

  /// registration macro for configurables
  /** @see \ref REG */
#define REGISTER_CONFIGURABLE(NAME,CREATE)                              \
  struct StaticConfigurableRegistrationFor_##NAME{ \
    typedef StaticConfigurableRegistrationFor_##NAME This;              \
    static Configurable *create(){                                      \
      CREATE;                                                           \
    }                                                                   \
    StaticConfigurableRegistrationFor_##NAME(){                         \
      Configurable::register_configurable_type(#NAME, &This::create);   \
    }                                                                   \
  } staticConfigurableRegistrationFor_##NAME;


  /// simpel registration macro for configurables that provide a default constructor
  /** @see \ref REG */
#define REGISTER_CONFIGURABLE_DEFAULT(NAME) REGISTER_CONFIGURABLE(NAME,return new NAME)

  } // namespace icl::utils