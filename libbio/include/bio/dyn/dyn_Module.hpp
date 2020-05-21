
#pragma once
#include <bio/dyn/dyn_Relocation.hpp>
#include <bio/mem/mem_SharedObject.hpp>
#include <bio/util/util_Templates.hpp>

namespace bio::dyn {

    enum class ModuleState {
        Invalid,
        Queued,
        Scanned,
        Relocated,
        Initialized,
        Finalized,
        Unloaded
    };

    struct ModuleInput {
        void *base;
        void *nro;
        void *nrr;
        void *bss;
        void *loader_data;
        bool is_nro;
        bool is_global;
        bool has_run_basic_relocations;

        constexpr ModuleInput() : base(nullptr), nro(nullptr), nrr(nullptr), bss(nullptr), loader_data(nullptr), is_nro(false), is_global(false), has_run_basic_relocations(false) {}

        inline constexpr bool IsValid() {
            if(this->base != nullptr) {
                if(this->is_nro) {
                    // Check that all its buffers are fine
                    if(this->nro != nullptr) {
                        if(this->nrr != nullptr) {
                            if(this->bss != nullptr) {
                                return true;
                            }
                        }
                    }
                }
                else {
                    // For raw memory modules, this is the only check to do
                    return true;
                }
            }
            return false;
        }

    };

    class Module;
    
    Result LoadRawModule(void *base, mem::SharedObject<Module> &out_module);
    Result LoadNroModule(void *nro_buf, u64 nro_size, bool is_global, mem::SharedObject<Module> &out_module);

    class Module {
    
        private:
            friend Result LoadRawModule(void *base, mem::SharedObject<Module> &out_module);
            friend Result LoadNroModule(void *nro_buf, u64 nro_size, bool is_global, mem::SharedObject<Module> &out_module);

        private:
            ModuleState state;
            ModuleInput input;
            // std::vector<Module> dependencies;
            elf::Dyn *dynamic;
            elf::Sym *symtab;
            char *strtab;
            u32 *hash;

            Result LoadBase();
            Result LoadFromNro(void *nro_data, u64 nro_data_size, bool is_global);
            Result LoadRaw(void *base);

            Result Scan();
            Result TryResolveSymbol(const char *find_name, u64 find_name_hash, elf::Sym *&def, Module *defining_module_ptr, bool require_global);
            Result ResolveLoadSymbol(const char *find_name, elf::Sym *&def, Module *defining_module_ptr);
            Result ResolveDependencySymbol(const char *find_name, elf::Sym *&def, Module *defining_module_ptr);
            Result RunRelocationTable(elf::Tag offset_tag, elf::Tag size_tag);
            Result Relocate();
            Result Initialize();
            Result Finalize();
            void Unload();
            
            inline void Destroy() {
                this->Finalize();
                this->Unload();
            }

            Result ResolveSymbolBase(const char *name, void *&out_symbol);

        public:
            Module() : state(ModuleState::Invalid), input(), /*dependencies(),*/ dynamic(nullptr), symtab(nullptr), strtab(nullptr), hash(nullptr) {}

            ~Module() {
                // This won't do anything if the module isn't loaded (empty/non-initialized instances don't have anything to dispose)
                this->Destroy();
            }

            inline constexpr bool IsValid() {
                return (this->state != ModuleState::Invalid) && this->input.IsValid();
            }

            template<typename F>
            inline Result ResolveSymbol(const char *name, F &out_symbol) {
                static_assert(util::IsPointer<F>, "Invalid symbol type - must be a function pointer, like void(*)()");
                return this->ResolveSymbolBase(name, reinterpret_cast<void*&>(out_symbol));
            }

    };

}