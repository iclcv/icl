
#include <memory>
#include <string>
#include <unordered_map>

#include <ICLCore/Img.h>

namespace icl::filter{
    enum class Backend{
        Cpp,
        Simd,
        OpenCL,
        Ipp,
    };

    struct AlgoBase{
        std::string name;
        virtual bool supports(depth d, const core::ImgParams& params) const = 0;
        virtual ~AlgoBase() = default;
        template<class... Args>
        auto operator()(Args&&... args){         // ?? how do we do the dispatching here?
            return applyBase(std::forward<Args>(args)...);
        }
    }
    using AlgoPitr = std::unique_ptr<AlgoBase>;
    template<class F>
    struct Algo : public AlgoBase{
        F f;
        template<class R, class... Args>
        R call()(Args&&... args){
            return f(std::forward<Args>(args)...);
        }
    };


    class AlgoSwitchBase{
        // some introspection
        
        //Destructor
        virtual ~AlgoSwitchBase() = default;
    }

    template<class T>
    struct AlgoSwitch : public AlgoSwitchBase{
        std::unordered_map<Backend, AlgoPtr> algos;
        AlgoPtr getBest(depth d, const core::ImgParams& params){
            // some dispatching logic to select the best algo for the given parameters
            // e.g. check if simd is supported for the given depth and parameters, if yes return simd algo, else cpp algo
        }
        AlgoPtr get(Backend b){
            return algos.at(b); // or nulllptr
        }
    }

    struct DispatchingFilter{
        virtual ~DispatchingFilter() = default;
        // for introspection ..
        virtual std::vector<AlgoSwitchBase*> switches() = 0;

    }

    struct MyFilter : public UnaryOp, public DispatchingFilter{
        AlgoSwitch<?> preprocessing;
        AlgoSwitch<?> main;
        AlgoSwitch<?> postprocessing;

        MyFilter(){
            // alternatively, it would possibly nice if there was a global registry (with names like "filter.algo" where an algo could register itself from a cpp-file without having to be included here 
            preprocess.algos["cpp"] = std::make_unique<Algo<?>>(/* cpp implementation from different file .. not sure how the templating should work though */);
        }

        virtual std::vector<AlgoSwitchBase*> switches() override{
            return {&preprocessing, &main, &postprocessing};
        }

        void apply(const core::Image &src, core::Image &dst) override{
            // dispatching logic
            auto preproc_algo = preprocessing.algos["cpp"]; // some dispatching logic to select the algo

        }

        


    };


}