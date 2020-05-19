#include <bio/dyn/dyn_Module.hpp>
#include <bio/dyn/dyn_Results.hpp>
// #include <bio/service/ro/ro_RoService.hpp>

namespace bio::dyn {

    constexpr u32 MaxModules = 16;

    static mem::SharedObject<Module> g_ModuleList[MaxModules];

    namespace {

        void PushModule(mem::SharedObject<Module> mod) {
            for(u32 i = 0; i < MaxModules; i++) {
                if(!g_ModuleList[i].IsValid()) {
                    g_ModuleList[i] = mod;
                    break;
                }
            }
        }

        void RemoveModule(mem::SharedObject<Module> mod) {
            for(u32 i = 0; i < MaxModules; i++) {
                if(g_ModuleList[i] == mod) {
                    g_ModuleList[i] = nullptr;
                    break;
                }
            }
        }

    }
    
    Result LoadRawModule(void *base, mem::SharedObject<Module> &out_module) {
        auto mod = mem::NewShared<Module>();
        RES_TRY(mod->LoadRaw(base));
        RES_TRY(mod->LoadBase());

        PushModule(mod);
        out_module = mod;
        return ResultSuccess;
    }

    Result LoadNroModule(void *nro_buf, u64 nro_size, bool is_global, mem::SharedObject<Module> &out_module) {
        auto mod = mem::NewShared<Module>();
        RES_TRY(mod->LoadFromNro(nro_buf, nro_size, is_global));
        RES_TRY(mod->LoadBase());

        PushModule(mod);
        out_module = mod;
        return ResultSuccess;
    }

    void UnloadModule(mem::SharedObject<Module> &mod) {
        RemoveModule(mod);
    }

    Result Module::LoadBase() {
        RET_UNLESS(this->state == ModuleState::Queued, result::ResultInvalidModuleState);
        RES_TRY(this->Scan());
        RES_TRY(this->Relocate());
        RES_TRY(this->Initialize());
        return ResultSuccess;
    }

    Result Module::LoadFromNro(void *nro_data, u64 nro_data_size, bool is_global) {
        /*
        RET_UNLESS(service::ro::IsInitialized(), 0xbeef);
        RET_UNLESS(util::IsAddressAligned(nro_data, util::PageAlignValue), 0xdead2);

        auto nro_header = reinterpret_cast<NROHeader*>(nro_data);
        const auto bss_size = nro_header->bss_size;
        RET_UNLESS(bss_size > 0, 0xdead3);

        // TODO: find a proper way to get the program ID on >3.0.0, when this svc info type didn't exist
        // Default to album/hbl in case we aren't able to get it
        u64 cur_program_id = 0x010000000000100D;
        svc::GetInfo(cur_program_id, 18, svc::CurrentProcessPseudoHandle, 0);

        const auto nrr_size = util::AlignUp(NRRHeader::GetNRRSize(1), util::PageAlignValue);
        auto nrr_buf = memalign(0x1000, nrr_size);//util::NewAligned(nrr_size, util::PageAlignValue);
        RET_UNLESS(nrr_buf != nullptr, 0xdead3);
        NRRHeader::CreateNRRHeader(nrr_buf, nrr_size, cur_program_id, 1);
        NRRHeader::SetNROHashAt(nrr_buf, nro_data, nro_data_size, 0);

        auto bss_buf = memalign(0x1000, bss_size);//util::NewAligned(bss_size, util::PageAlignValue);
        RET_UNLESS(bss_buf != nullptr, 0xdead3);

        RES_TRY(service::ro::RoServiceSession->LoadNrr(nrr_buf, nrr_size));

        u64 nro_addr = 0;
        RES_TRY(service::ro::RoServiceSession->LoadNro(nro_data, nro_data_size, bss_buf, bss_size, nro_addr));

        this->input.nro = nro_data;
        this->input.nrr = nrr_buf;
        this->input.bss = bss_buf;
        this->input.is_nro = true;
        this->input.is_global = is_global;
        this->input.has_run_basic_relocations = false;
        this->input.base = reinterpret_cast<void*>(nro_addr);
        this->state = ModuleState::Queued;
        return ResultSuccess;
        */
        return result::ResultInvalidInput;
    }

    Result Module::LoadRaw(void *base) {
        RET_UNLESS(base != nullptr, 0xdead4);

        this->input.nro = nullptr;
        this->input.nrr = nullptr;
        this->input.bss = nullptr;
        this->input.is_nro = false;
        this->input.has_run_basic_relocations = true;
        this->input.is_global = true;
        this->input.base = base;
        this->state = ModuleState::Queued;
        return ResultSuccess;
    }

    Result Module::Scan() {
        RET_UNLESS(this->input.IsValid(), result::ResultInvalidInput);
        auto module_base = reinterpret_cast<u8*>(this->input.base);
        auto module_start = reinterpret_cast<nro::HeaderStart*>(module_base);
        auto mod_header = reinterpret_cast<mod::Header*>(module_base + module_start->module_header_offset);
        this->dynamic = reinterpret_cast<elf::Dyn*>(reinterpret_cast<u8*>(mod_header) + mod_header->dynamic);
        RET_UNLESS(mod_header->magic == mod::MOD0, result::ResultInvalidInput);

        void *hash_v = nullptr;
        RES_TRY_EXCEPT(this->dynamic->FindOffset(elf::Tag::Hash, hash_v, module_base), result::ResultMissingDtEntry);
        this->hash = reinterpret_cast<u32*>(hash_v);

        void *strtab_v = nullptr;
        RES_TRY_EXCEPT(this->dynamic->FindOffset(elf::Tag::StrTab, strtab_v, module_base), result::ResultMissingDtEntry);
        this->strtab = reinterpret_cast<char*>(strtab_v);

        void *symtab_v = nullptr;
        RES_TRY_EXCEPT(this->dynamic->FindOffset(elf::Tag::SymTab, symtab_v, module_base), result::ResultMissingDtEntry);
        this->symtab = reinterpret_cast<elf::Sym*>(symtab_v);

        u64 syment = 0;
        auto rc = this->dynamic->FindValue(elf::Tag::SymEnt, syment);
        RES_TRY_EXCEPT(rc, result::ResultMissingDtEntry);
        if(rc.IsSuccess()) {
            RET_UNLESS(syment == sizeof(elf::Sym), result::ResultInvalidSymEnt);
        }

        /*
        for(auto walker = this->dynamic; walker->tag != 0; walker++) {
            if(walker->tag == 1) {
                // TODO: handle dependencies
            }
        }
        */

        this->state = ModuleState::Scanned;
        return ResultSuccess;
    }

    int strcmp(const char *s1, const char *s2) {
        while (*s1 != '\0' && *s2 != '\0'  && *s1 == *s2) {
            s1++;
            s2++;
        }
        return *s1 - *s2;
    }

    Result Module::TryResolveSymbol(const char *find_name, u64 find_name_hash, elf::Sym *&def, Module *defining_module_ptr, bool require_global){
        RET_IF(require_global && !this->input.is_global, result::ResultCouldNotResolveSymbol);
        RET_UNLESS(this->symtab != nullptr, result::ResultCouldNotResolveSymbol);
        RET_UNLESS(this->strtab != nullptr, result::ResultCouldNotResolveSymbol);
        RET_UNLESS(this->hash != nullptr, result::ResultCouldNotResolveSymbol);
        
        auto nbucket = this->hash[0];
        auto index = this->hash[2 + (find_name_hash % nbucket)];
        auto chains = this->hash + 2 + nbucket;
        while((index != 0) && (strcmp(find_name, this->strtab + this->symtab[index].name) != 0)) {
            index = chains[index];
        }

        RET_UNLESS(index != 0, result::ResultCouldNotResolveSymbol);

        auto sym = &this->symtab[index];
        RET_UNLESS(sym->shndx != 0, result::ResultCouldNotResolveSymbol);

        def = sym;
        defining_module_ptr = this;
        return ResultSuccess;
    }

    Result Module::ResolveLoadSymbol(const char *find_name, elf::Sym *&def, Module *defining_module_ptr) {
        auto hash = elf::HashString(find_name);

        for(auto &mod: g_ModuleList) {
            RES_TRY_EXCEPT(mod->TryResolveSymbol(find_name, hash, def, defining_module_ptr, true), result::ResultCouldNotResolveSymbol);
        }

        return this->TryResolveSymbol(find_name, hash, def, defining_module_ptr, false);
    }

    Result Module::ResolveDependencySymbol(const char *find_name, elf::Sym *&def, Module *defining_module_ptr) {
        auto hash = elf::HashString(find_name);
        auto rc = this->TryResolveSymbol(find_name, hash, def, defining_module_ptr, false);
        RES_TRY_EXCEPT(rc, result::ResultCouldNotResolveSymbol);
        if(rc.IsSuccess()) {
            return rc;
        }

        /*
        for(auto &dep: this->dependencies) {
            rc = dep.TryResolveSymbol(find_name, hash, def, defining_module_ptr, false);
            RES_TRY_EXCEPT(rc, result::ResultCouldNotResolveSymbol);
            if(rc.IsSuccess()) {
                return rc;
            }
        }

        for(auto &dep: this->dependencies) {
            rc = dep.ResolveDependencySymbol(find_name, def, defining_module_ptr);
            RES_TRY_EXCEPT(rc, result::ResultCouldNotResolveSymbol);
            if(rc.IsSuccess()) {
                return rc;
            }
        }
        */

        return ResultSuccess;
    }

    Result Module::RunRelocationTable(elf::Tag offset_tag, elf::Tag size_tag) {
        void *raw_table = nullptr;
        auto rc = this->dynamic->FindOffset(offset_tag, raw_table, this->input.base);
        RES_TRY_EXCEPT(rc, result::ResultMissingDtEntry);
        RET_UNLESS(rc.IsSuccess(), ResultSuccess);

        u64 table_size = 0;
        auto table_type = offset_tag;
        RES_TRY(this->dynamic->FindValue(size_tag, table_size));

        if(offset_tag == elf::Tag::JmpRel) {
            u64 tmp_type = 0;
            RES_TRY(this->dynamic->FindValue(elf::Tag::PltRel, tmp_type));
            table_type = static_cast<elf::Tag>(tmp_type);
        }

        u64 ent_size = 0;
        switch(table_type)
        {
            case elf::Tag::RelaOffset: {
                rc = this->dynamic->FindValue(elf::Tag::RelEntrySize, ent_size);
                RES_TRY_EXCEPT(rc, result::ResultMissingDtEntry);
                if(rc.IsSuccess()) {
                    RET_UNLESS(ent_size == sizeof(elf::Rela), result::ResultInvalidRelocEnt);
                }
                else {
                    ent_size = sizeof(elf::Rela);
                }
                break;
            }
            case elf::Tag::RelOffset: {
                rc = this->dynamic->FindValue(elf::Tag::RelEntrySize, ent_size);
                RES_TRY_EXCEPT(rc, result::ResultMissingDtEntry);
                if(rc.IsSuccess()) {
                    RET_UNLESS(ent_size == sizeof(elf::Rel), result::ResultInvalidRelocEnt);
                }
                else {
                    ent_size = sizeof(elf::Rel);
                }
                break;
            }
            default:
                return result::ResultInvalidRelocTableType;
        }

        RET_UNLESS((table_size % ent_size) == 0, result::ResultInvalidRelocTableSize);

        auto raw_table8 = reinterpret_cast<u8*>(raw_table);
        for(u64 offset = 0; offset < table_size; offset += ent_size) {
            elf::Rela rela = {};
            switch(table_type) {
                case elf::Tag::RelaOffset: {
                    auto rela_buf = reinterpret_cast<elf::Rela*>(raw_table8 + offset);
                    rela = *rela_buf;
                    break;
                }
                case elf::Tag::RelOffset: {
                    auto rela_buf = reinterpret_cast<elf::Rel*>(raw_table8 + offset);
                    rela.offset = rela_buf->offset;
                    rela.info = rela_buf->info;
                    break;
                }
                default:
                    break;
            }

            auto base8 = reinterpret_cast<u8*>(this->input.base);

            auto mod = this;
            auto mod_base8 = reinterpret_cast<u8*>(mod->input.base);
            u8 *symbol = nullptr;
            if(rela.info.symbol != 0) {
                RET_UNLESS(this->symtab != nullptr, result::ResultNeedsSymTab);
                RET_UNLESS(this->strtab != nullptr, result::ResultNeedsStrTab);

                auto sym = &this->symtab[rela.info.symbol];
                
                elf::Sym *def = nullptr;
                auto rc = this->ResolveLoadSymbol(this->strtab + sym->value, def, mod);
                if(rc.IsFailure()) {
                    continue;
                }
                mod_base8 = reinterpret_cast<u8*>(mod->input.base);
                symbol = mod_base8 + def->value;
            }
            
            switch(static_cast<elf::RelocationType>(rela.info.reloc_type))
            {
                case elf::RelocationType::AArch64Abs64:
                case elf::RelocationType::AArch64GlobDat:
                case elf::RelocationType::AArch64JumpSlot: {
                    auto target = reinterpret_cast<void**>(base8 + rela.offset);
                    if(table_type == elf::Tag::RelOffset) {
                        rela.addend = reinterpret_cast<u64>(*target);
                    }
                    *target = symbol + rela.addend;
                    break;
                }
                case elf::RelocationType::AArch64Relative: {
                    if(!this->input.has_run_basic_relocations) {
                        auto target = reinterpret_cast<void**>(base8 + rela.offset);
                        if(table_type == elf::Tag::RelOffset) {
                            rela.addend = reinterpret_cast<u64>(*target);
                        }
                        *target = mod_base8 + rela.addend;
                    }
                    break;
                }
                default:
                    return result::ResultUnrecognizedRelocType;
            }
        }
        return ResultSuccess;
    }

    Result Module::Relocate() {
        RES_TRY(this->RunRelocationTable(elf::Tag::RelaOffset, elf::Tag::RelaSize));
        RES_TRY(this->RunRelocationTable(elf::Tag::RelOffset, elf::Tag::RelSize));
        RES_TRY(this->RunRelocationTable(elf::Tag::JmpRel, elf::Tag::PltRelSize));

        this->state = ModuleState::Relocated;
        return ResultSuccess;
    }

    Result Module::Initialize() {
        RET_UNLESS(this->state == ModuleState::Relocated, result::ResultInvalidModuleState);
        
        void *init_array_ptr = nullptr;
        u64 init_array_size = 0;

        // Find init array, return success if not present, call it otherwise
        auto rc = this->dynamic->FindOffset(elf::Tag::InitArray, init_array_ptr, this->input.base);
        RES_TRY_EXCEPT(rc, result::ResultMissingDtEntry);
        RET_UNLESS(rc.IsSuccess(), ResultSuccess);
        RES_TRY(this->dynamic->FindValue(elf::Tag::InitArraySize, init_array_size));

        auto init_array = reinterpret_cast<void(**)()>(init_array_ptr);

        const u64 init_count = init_array_size / sizeof(init_array[0]);
        for(u64 i = 0; i < init_count; i++) {
            init_array[i]();
        }

        this->state = ModuleState::Initialized;
        return ResultSuccess;
    }

    Result Module::Finalize() {
        RET_UNLESS(this->state == ModuleState::Initialized, result::ResultInvalidModuleState);
        
        void *fini_array_ptr = nullptr;
        u64 fini_array_size = 0;

        // Find fini array, return success if not present, call it otherwise
        auto rc = this->dynamic->FindOffset(elf::Tag::FiniArray, fini_array_ptr, this->input.base);
        RES_TRY_EXCEPT(rc, result::ResultMissingDtEntry);
        RET_UNLESS(rc.IsSuccess(), ResultSuccess);
        RES_TRY(this->dynamic->FindValue(elf::Tag::FiniArraySize, fini_array_size));

        auto fini_array = reinterpret_cast<void(**)()>(fini_array_ptr);

        const u64 fini_count = fini_array_size / sizeof(fini_array[0]);
        for(u64 i = 0; i < fini_count; i++) {
            fini_array[i]();
        }

        this->state = ModuleState::Finalized;
        return ResultSuccess;
    }

    void Module::Unload() {
        // We only close/dispose with NROs
        if(this->input.is_nro) {
            /*
            if(this->input.IsValid()) {
                if(service::ro::IsInitialized()) {
                    service::ro::RoServiceSession->UnloadNro(this->input.base);
                    service::ro::RoServiceSession->UnloadNrr(this->input.nrr);
                }
                util::DeleteAligned(this->input.nrr, util::PageAlignValue);
                util::DeleteAligned(this->input.bss, util::PageAlignValue);
            }
            */
        }
    }

    Result Module::ResolveSymbolBase(const char *name, void *&out_symbol) {
        elf::Sym *def = nullptr;
        auto def_mod_ptr = this;
        RES_TRY(this->ResolveDependencySymbol(name, def, def_mod_ptr));

        out_symbol = reinterpret_cast<void*>(reinterpret_cast<u8*>(def_mod_ptr->input.base) + def->value);
        return ResultSuccess;
    }

}
