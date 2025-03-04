/*************************************************************************/
/*  editor_export.h                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef EDITOR_EXPORT_H
#define EDITOR_EXPORT_H

#include "core/io/dir_access.h"
#include "core/io/resource.h"
#include "scene/main/node.h"
#include "scene/main/timer.h"
#include "scene/resources/texture.h"

class FileAccess;
class EditorExportPlatform;
class EditorFileSystemDirectory;
struct EditorProgress;

class EditorExportPreset : public RefCounted {
	GDCLASS(EditorExportPreset, RefCounted);

public:
	enum ExportFilter {
		EXPORT_ALL_RESOURCES,
		EXPORT_SELECTED_SCENES,
		EXPORT_SELECTED_RESOURCES,
		EXCLUDE_SELECTED_RESOURCES,
	};

	enum ScriptExportMode {
		MODE_SCRIPT_TEXT,
		MODE_SCRIPT_COMPILED,
	};

private:
	Ref<EditorExportPlatform> platform;
	ExportFilter export_filter = EXPORT_ALL_RESOURCES;
	String include_filter;
	String exclude_filter;
	String export_path;

	String exporter;
	HashSet<String> selected_files;
	bool runnable = false;

	friend class EditorExport;
	friend class EditorExportPlatform;

	List<PropertyInfo> properties;
	HashMap<StringName, Variant> values;

	String name;

	String custom_features;

	String enc_in_filters;
	String enc_ex_filters;
	bool enc_pck = false;
	bool enc_directory = false;

	int script_mode = MODE_SCRIPT_COMPILED;
	String script_key;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	Ref<EditorExportPlatform> get_platform() const;

	bool has(const StringName &p_property) const { return values.has(p_property); }

	void update_files_to_export();

	Vector<String> get_files_to_export() const;

	void add_export_file(const String &p_path);
	void remove_export_file(const String &p_path);
	bool has_export_file(const String &p_path);

	void set_name(const String &p_name);
	String get_name() const;

	void set_runnable(bool p_enable);
	bool is_runnable() const;

	void set_export_filter(ExportFilter p_filter);
	ExportFilter get_export_filter() const;

	void set_include_filter(const String &p_include);
	String get_include_filter() const;

	void set_exclude_filter(const String &p_exclude);
	String get_exclude_filter() const;

	void set_custom_features(const String &p_custom_features);
	String get_custom_features() const;

	void set_export_path(const String &p_path);
	String get_export_path() const;

	void set_enc_in_filter(const String &p_filter);
	String get_enc_in_filter() const;

	void set_enc_ex_filter(const String &p_filter);
	String get_enc_ex_filter() const;

	void set_enc_pck(bool p_enabled);
	bool get_enc_pck() const;

	void set_enc_directory(bool p_enabled);
	bool get_enc_directory() const;

	void set_script_export_mode(int p_mode);
	int get_script_export_mode() const;

	void set_script_encryption_key(const String &p_key);
	String get_script_encryption_key() const;

	const List<PropertyInfo> &get_properties() const { return properties; }

	EditorExportPreset() {}
};

struct SharedObject {
	String path;
	Vector<String> tags;
	String target;

	SharedObject(const String &p_path, const Vector<String> &p_tags, const String &p_target) :
			path(p_path),
			tags(p_tags),
			target(p_target) {
	}

	SharedObject() {}
};

class EditorExportPlatform : public RefCounted {
	GDCLASS(EditorExportPlatform, RefCounted);

public:
	typedef Error (*EditorExportSaveFunction)(void *p_userdata, const String &p_path, const Vector<uint8_t> &p_data, int p_file, int p_total, const Vector<String> &p_enc_in_filters, const Vector<String> &p_enc_ex_filters, const Vector<uint8_t> &p_key);
	typedef Error (*EditorExportSaveSharedObject)(void *p_userdata, const SharedObject &p_so);

private:
	struct SavedData {
		uint64_t ofs = 0;
		uint64_t size = 0;
		bool encrypted = false;
		Vector<uint8_t> md5;
		CharString path_utf8;

		bool operator<(const SavedData &p_data) const {
			return path_utf8 < p_data.path_utf8;
		}
	};

	struct PackData {
		Ref<FileAccess> f;
		Vector<SavedData> file_ofs;
		EditorProgress *ep = nullptr;
		Vector<SharedObject> *so_files = nullptr;
	};

	struct ZipData {
		void *zip = nullptr;
		EditorProgress *ep = nullptr;
	};

	struct FeatureContainers {
		HashSet<String> features;
		Vector<String> features_pv;
	};

	void _export_find_resources(EditorFileSystemDirectory *p_dir, HashSet<String> &p_paths);
	void _export_find_dependencies(const String &p_path, HashSet<String> &p_paths);

	void gen_debug_flags(Vector<String> &r_flags, int p_flags);
	static Error _save_pack_file(void *p_userdata, const String &p_path, const Vector<uint8_t> &p_data, int p_file, int p_total, const Vector<String> &p_enc_in_filters, const Vector<String> &p_enc_ex_filters, const Vector<uint8_t> &p_key);
	static Error _save_zip_file(void *p_userdata, const String &p_path, const Vector<uint8_t> &p_data, int p_file, int p_total, const Vector<String> &p_enc_in_filters, const Vector<String> &p_enc_ex_filters, const Vector<uint8_t> &p_key);

	void _edit_files_with_filter(Ref<DirAccess> &da, const Vector<String> &p_filters, HashSet<String> &r_list, bool exclude);
	void _edit_filter_list(HashSet<String> &r_list, const String &p_filter, bool exclude);

	static Error _add_shared_object(void *p_userdata, const SharedObject &p_so);

protected:
	struct ExportNotifier {
		ExportNotifier(EditorExportPlatform &p_platform, const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags);
		~ExportNotifier();
	};

	FeatureContainers get_feature_containers(const Ref<EditorExportPreset> &p_preset, bool p_debug);

	bool exists_export_template(String template_file_name, String *err) const;
	String find_export_template(String template_file_name, String *err = nullptr) const;
	void gen_export_flags(Vector<String> &r_flags, int p_flags);

public:
	virtual void get_preset_features(const Ref<EditorExportPreset> &p_preset, List<String> *r_features) = 0;

	struct ExportOption {
		PropertyInfo option;
		Variant default_value;

		ExportOption(const PropertyInfo &p_info, const Variant &p_default) :
				option(p_info),
				default_value(p_default) {
		}
		ExportOption() {}
	};

	virtual Ref<EditorExportPreset> create_preset();

	virtual void get_export_options(List<ExportOption> *r_options) = 0;
	virtual bool should_update_export_options() { return false; }
	virtual bool get_export_option_visibility(const String &p_option, const HashMap<StringName, Variant> &p_options) const { return true; }

	virtual String get_os_name() const = 0;
	virtual String get_name() const = 0;
	virtual Ref<Texture2D> get_logo() const = 0;

	Error export_project_files(const Ref<EditorExportPreset> &p_preset, bool p_debug, EditorExportSaveFunction p_func, void *p_udata, EditorExportSaveSharedObject p_so_func = nullptr);

	Error save_pack(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, Vector<SharedObject> *p_so_files = nullptr, bool p_embed = false, int64_t *r_embedded_start = nullptr, int64_t *r_embedded_size = nullptr);
	Error save_zip(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path);

	virtual bool poll_export() { return false; }
	virtual int get_options_count() const { return 0; }
	virtual String get_options_tooltip() const { return ""; }
	virtual Ref<ImageTexture> get_option_icon(int p_index) const;
	virtual String get_option_label(int p_device) const { return ""; }
	virtual String get_option_tooltip(int p_device) const { return ""; }

	enum DebugFlags {
		DEBUG_FLAG_DUMB_CLIENT = 1,
		DEBUG_FLAG_REMOTE_DEBUG = 2,
		DEBUG_FLAG_REMOTE_DEBUG_LOCALHOST = 4,
		DEBUG_FLAG_VIEW_COLLISONS = 8,
		DEBUG_FLAG_VIEW_NAVIGATION = 16,
	};

	virtual Error run(const Ref<EditorExportPreset> &p_preset, int p_device, int p_debug_flags) { return OK; }
	virtual Ref<Texture2D> get_run_icon() const { return get_logo(); }

	String test_etc2() const;
	virtual bool can_export(const Ref<EditorExportPreset> &p_preset, String &r_error, bool &r_missing_templates) const = 0;

	virtual List<String> get_binary_extensions(const Ref<EditorExportPreset> &p_preset) const = 0;
	virtual Error export_project(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags = 0) = 0;
	virtual Error export_pack(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags = 0);
	virtual Error export_zip(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags = 0);
	virtual void get_platform_features(List<String> *r_features) = 0;
	virtual void resolve_platform_feature_priorities(const Ref<EditorExportPreset> &p_preset, HashSet<String> &p_features) = 0;
	virtual String get_debug_protocol() const { return "tcp://"; }

	EditorExportPlatform();
};

class EditorExportPlugin : public RefCounted {
	GDCLASS(EditorExportPlugin, RefCounted);

	friend class EditorExportPlatform;

	Ref<EditorExportPreset> export_preset;

	Vector<SharedObject> shared_objects;
	struct ExtraFile {
		String path;
		Vector<uint8_t> data;
		bool remap = false;
	};
	Vector<ExtraFile> extra_files;
	bool skipped = false;

	Vector<String> ios_frameworks;
	Vector<String> ios_embedded_frameworks;
	Vector<String> ios_project_static_libs;
	String ios_plist_content;
	String ios_linker_flags;
	Vector<String> ios_bundle_files;
	String ios_cpp_code;

	Vector<String> osx_plugin_files;

	_FORCE_INLINE_ void _clear() {
		shared_objects.clear();
		extra_files.clear();
		skipped = false;
	}

	_FORCE_INLINE_ void _export_end() {
		ios_frameworks.clear();
		ios_embedded_frameworks.clear();
		ios_bundle_files.clear();
		ios_plist_content = "";
		ios_linker_flags = "";
		ios_cpp_code = "";
		osx_plugin_files.clear();
	}

	void _export_file_script(const String &p_path, const String &p_type, const Vector<String> &p_features);
	void _export_begin_script(const Vector<String> &p_features, bool p_debug, const String &p_path, int p_flags);
	void _export_end_script();

protected:
	void set_export_preset(const Ref<EditorExportPreset> &p_preset);
	Ref<EditorExportPreset> get_export_preset() const;

	void add_file(const String &p_path, const Vector<uint8_t> &p_file, bool p_remap);
	void add_shared_object(const String &p_path, const Vector<String> &tags, const String &p_target = String());

	void add_ios_framework(const String &p_path);
	void add_ios_embedded_framework(const String &p_path);
	void add_ios_project_static_lib(const String &p_path);
	void add_ios_plist_content(const String &p_plist_content);
	void add_ios_linker_flags(const String &p_flags);
	void add_ios_bundle_file(const String &p_path);
	void add_ios_cpp_code(const String &p_code);
	void add_osx_plugin_file(const String &p_path);

	void skip();

	virtual void _export_file(const String &p_path, const String &p_type, const HashSet<String> &p_features);
	virtual void _export_begin(const HashSet<String> &p_features, bool p_debug, const String &p_path, int p_flags);

	static void _bind_methods();

	GDVIRTUAL3(_export_file, String, String, Vector<String>)
	GDVIRTUAL4(_export_begin, Vector<String>, bool, String, uint32_t)
	GDVIRTUAL0(_export_end)

public:
	Vector<String> get_ios_frameworks() const;
	Vector<String> get_ios_embedded_frameworks() const;
	Vector<String> get_ios_project_static_libs() const;
	String get_ios_plist_content() const;
	String get_ios_linker_flags() const;
	Vector<String> get_ios_bundle_files() const;
	String get_ios_cpp_code() const;
	const Vector<String> &get_osx_plugin_files() const;

	EditorExportPlugin();
};

class EditorExport : public Node {
	GDCLASS(EditorExport, Node);

	Vector<Ref<EditorExportPlatform>> export_platforms;
	Vector<Ref<EditorExportPreset>> export_presets;
	Vector<Ref<EditorExportPlugin>> export_plugins;

	StringName _export_presets_updated;

	Timer *save_timer = nullptr;
	bool block_save = false;

	static EditorExport *singleton;

	void _save();

protected:
	friend class EditorExportPreset;
	void save_presets();

	void _notification(int p_what);
	static void _bind_methods();

public:
	static EditorExport *get_singleton() { return singleton; }

	void add_export_platform(const Ref<EditorExportPlatform> &p_platform);
	int get_export_platform_count();
	Ref<EditorExportPlatform> get_export_platform(int p_idx);

	void add_export_preset(const Ref<EditorExportPreset> &p_preset, int p_at_pos = -1);
	int get_export_preset_count() const;
	Ref<EditorExportPreset> get_export_preset(int p_idx);
	void remove_export_preset(int p_idx);

	void add_export_plugin(const Ref<EditorExportPlugin> &p_plugin);
	void remove_export_plugin(const Ref<EditorExportPlugin> &p_plugin);
	Vector<Ref<EditorExportPlugin>> get_export_plugins();

	void load_config();
	void update_export_presets();
	bool poll_export_platforms();

	EditorExport();
	~EditorExport();
};

class EditorExportPlatformPC : public EditorExportPlatform {
	GDCLASS(EditorExportPlatformPC, EditorExportPlatform);

private:
	Ref<ImageTexture> logo;
	String name;
	String os_name;

	int chmod_flags = -1;

public:
	virtual void get_preset_features(const Ref<EditorExportPreset> &p_preset, List<String> *r_features) override;

	virtual void get_export_options(List<ExportOption> *r_options) override;

	virtual String get_name() const override;
	virtual String get_os_name() const override;
	virtual Ref<Texture2D> get_logo() const override;

	virtual bool can_export(const Ref<EditorExportPreset> &p_preset, String &r_error, bool &r_missing_templates) const override;
	virtual Error export_project(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags = 0) override;
	virtual Error sign_shared_object(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path);
	virtual String get_template_file_name(const String &p_target, const String &p_arch) const = 0;

	virtual Error prepare_template(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags);
	virtual Error modify_template(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags) { return OK; };
	virtual Error export_project_data(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags);

	void set_extension(const String &p_extension, const String &p_feature_key = "default");
	void set_name(const String &p_name);
	void set_os_name(const String &p_name);

	void set_logo(const Ref<Texture2D> &p_logo);

	void add_platform_feature(const String &p_feature);
	virtual void get_platform_features(List<String> *r_features) override;
	virtual void resolve_platform_feature_priorities(const Ref<EditorExportPreset> &p_preset, HashSet<String> &p_features) override;

	int get_chmod_flags() const;
	void set_chmod_flags(int p_flags);

	virtual Error fixup_embedded_pck(const String &p_path, int64_t p_embedded_start, int64_t p_embedded_size) const {
		return Error::OK;
	}
};

class EditorExportTextSceneToBinaryPlugin : public EditorExportPlugin {
	GDCLASS(EditorExportTextSceneToBinaryPlugin, EditorExportPlugin);

public:
	virtual void _export_file(const String &p_path, const String &p_type, const HashSet<String> &p_features) override;
	EditorExportTextSceneToBinaryPlugin();
};

#endif // EDITOR_IMPORT_EXPORT_H
