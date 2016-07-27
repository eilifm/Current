/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2016 Maxim Zhurovich <zhurovich@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#ifndef CURRENT_STORAGE_API_TYPES_H
#define CURRENT_STORAGE_API_TYPES_H

#include "storage.h"
#include "container/sfinae.h"

#include "../Blocks/HTTP/api.h"

namespace current {
namespace storage {
namespace rest {

const std::string kRESTfulDataURLComponent = "data";
const std::string kRESTfulSchemaURLComponent = "schema";

// TODO(dkorolev): The whole `FieldTypeDependentImpl` section below to be moved to `semantics.h`.
template <typename>
struct FieldTypeDependentImpl {};

template <>
struct FieldTypeDependentImpl<semantics::primary_key::Key> {
  using url_key_t = std::string;
  template <typename F_WITH, typename F_WITHOUT>
  static void CallWithOrWithoutKeyFromURL(Request&& request_rref, F_WITH&& with, F_WITHOUT&& without) {
    // TODO(dkorolev): Also accept `?row` w/o `?col`, and other types of one-of-two-keys scenarios,
    // as this `semantics::primary_key::Key` mode is used for partial access as well.
    if (request_rref.url_path_args.size() == 1u) {
      with(std::move(request_rref), request_rref.url_path_args[0]);
    } else if (request_rref.url.query.has("key")) {
      with(std::move(request_rref), request_rref.url.query["key"]);
    } else {
      without(std::move(request_rref));
    }
  }
  static std::string FormatURLKey(const std::string& key) {
    // TODO(dkorolev): Perhaps return "?key=..." in case the key contains slashes?
    return key;
  }
  template <typename KEY>
  static KEY ParseURLKey(const std::string& url_key) {
    return current::FromString<KEY>(url_key);
  }
  template <typename KEY>
  static std::string ComposeURLKey(const KEY& key) {
    return FormatURLKey(current::ToString(key));
  }
  template <typename RECORD>
  static auto ExtractOrComposeKey(const RECORD& entry)
      -> decltype(current::storage::sfinae::GetKey(std::declval<RECORD>())) {
    return current::storage::sfinae::GetKey(entry);
  }
};

template <>
struct FieldTypeDependentImpl<semantics::primary_key::RowCol> {
  using url_key_t = std::pair<std::string, std::string>;

  template <typename F_WITH, typename F_WITHOUT>
  static void CallWithOrWithoutKeyFromURL(Request&& request_rref, F_WITH&& with, F_WITHOUT&& without) {
    // TODO(dkorolev): `?1=...&2=...`, `?L=...&R=...`, etc. -- make the code more generic.
    if (request_rref.url_path_args.size() == 2u) {
      with(std::move(request_rref),
           std::make_pair(request_rref.url_path_args[0], request_rref.url_path_args[1]));
    } else if (request_rref.url.query.has("key1") && request_rref.url.query.has("key2")) {
      with(std::move(request_rref),
           std::make_pair(request_rref.url.query["key1"], request_rref.url.query["key2"]));
    } else if (request_rref.url.query.has("row") && request_rref.url.query.has("col")) {
      with(std::move(request_rref),
           std::make_pair(request_rref.url.query["row"], request_rref.url.query["col"]));
    } else {
      without(std::move(request_rref));
    }
  }
  static std::string FormatURLKey(const url_key_t& key) {
    // TODO(dkorolev): Perhaps return "?row=...&col=" in case the key contains slashes?
    return key.first + '/' + key.second;
  }
  template <typename KEY, typename URL_KEY>
  static KEY ParseURLKey(const URL_KEY& url_key) {
    return KEY(current::FromString<current::storage::sfinae::entry_row_t<KEY>>(url_key.first),
               current::FromString<current::storage::sfinae::entry_col_t<KEY>>(url_key.second));
  }
  template <typename KEY>
  static std::string ComposeURLKey(const KEY& key) {
    return FormatURLKey(std::make_pair(current::ToString(current::storage::sfinae::GetRow(key)),
                                       current::ToString(current::storage::sfinae::GetCol(key))));
  }
  template <typename RECORD>
  static auto ExtractOrComposeKey(const RECORD& entry)
      -> std::pair<decltype(current::storage::sfinae::GetRow(std::declval<RECORD>())),
                   decltype(current::storage::sfinae::GetCol(std::declval<RECORD>()))> {
    return std::make_pair(current::storage::sfinae::GetRow(entry), current::storage::sfinae::GetCol(entry));
  }
};

template <typename PRIMARY_KEY_TYPE>
struct FieldTypeDependent : FieldTypeDependentImpl<PRIMARY_KEY_TYPE> {
  template <typename F>
  static void CallWithKeyFromURL(Request&& request_rref, F&& next_with_key) {
    FieldTypeDependentImpl<PRIMARY_KEY_TYPE>::CallWithOrWithoutKeyFromURL(
        std::move(request_rref),
        std::forward<F>(next_with_key),
        [](Request&& proxied_request_rref) {
          proxied_request_rref("Need resource key in the URL.\n", HTTPResponseCode.BadRequest);
        });
  }

  template <typename F>
  static void CallWithOptionalKeyFromURL(Request&& request_rref, F&& next) {
    FieldTypeDependentImpl<PRIMARY_KEY_TYPE>::CallWithOrWithoutKeyFromURL(
        std::move(request_rref),
        std::forward<F>(next),
        [&next](Request&& proxied_request_rref) { next(std::move(proxied_request_rref), nullptr); });
  }
};

template <typename KEY>
using key_type_dependent_t = FieldTypeDependent<KEY>;

template <typename FIELD>
using field_type_dependent_t = key_type_dependent_t<typename FIELD::semantics_t::primary_key_t>;
// TODO(dkorolev): The whole `FieldTypeDependentImpl` section above to be moved to `semantics.h`.

template <typename STORAGE>
struct RESTfulGenericInput {
  using STORAGE_TYPE = STORAGE;
  STORAGE& storage;
  const std::string restful_url_prefix;

  explicit RESTfulGenericInput(STORAGE& storage) : storage(storage) {}
  RESTfulGenericInput(STORAGE& storage, const std::string& restful_url_prefix)
      : storage(storage), restful_url_prefix(restful_url_prefix) {}
  RESTfulGenericInput(const RESTfulGenericInput&) = default;
  RESTfulGenericInput(RESTfulGenericInput&&) = default;
};

template <typename STORAGE>
struct RESTfulRegisterTopLevelInput : RESTfulGenericInput<STORAGE> {
  const int port;
  HTTPRoutesScope& scope;
  const std::vector<std::string> field_names;
  const std::string route_prefix;
  std::atomic_bool& up_status;

  RESTfulRegisterTopLevelInput(STORAGE& storage,
                               const std::string& restful_url_prefix,
                               int port,
                               HTTPRoutesScope& scope,
                               const std::vector<std::string>& field_names,
                               const std::string& route_prefix,
                               std::atomic_bool& up_status)
      : RESTfulGenericInput<STORAGE>(storage, restful_url_prefix),
        port(port),
        scope(scope),
        field_names(field_names),
        route_prefix(route_prefix),
        up_status(up_status) {}
  RESTfulRegisterTopLevelInput(const RESTfulGenericInput<STORAGE>& input,
                               int port,
                               HTTPRoutesScope& scope,
                               const std::vector<std::string>& field_names,
                               const std::string& route_prefix,
                               std::atomic_bool& up_status)
      : RESTfulGenericInput<STORAGE>(input),
        port(port),
        scope(scope),
        field_names(field_names),
        route_prefix(route_prefix),
        up_status(up_status) {}
  RESTfulRegisterTopLevelInput(RESTfulGenericInput<STORAGE>&& input,
                               int port,
                               HTTPRoutesScope& scope,
                               const std::vector<std::string>& field_names,
                               const std::string& route_prefix,
                               std::atomic_bool& up_status)
      : RESTfulGenericInput<STORAGE>(std::move(input)),
        port(port),
        scope(scope),
        field_names(field_names),
        route_prefix(route_prefix),
        up_status(up_status) {}
};

template <typename STORAGE, typename FIELD>
struct RESTfulGETInput : RESTfulGenericInput<STORAGE> {
  using key_completeness_t = semantics::key_completeness::FullKey;

  using immutable_fields_t = ImmutableFields<STORAGE>;
  using url_key_t = typename FIELD::semantics_t::url_key_t;

  immutable_fields_t fields;
  const FIELD& field;
  const std::string field_name;
  Optional<url_key_t> get_url_key;
  const StorageRole role;
  const bool export_requested;

  RESTfulGETInput(const RESTfulGenericInput<STORAGE>& input,
                  immutable_fields_t fields,
                  const FIELD& field,
                  const std::string& field_name,
                  const Optional<url_key_t>& get_url_key,
                  const StorageRole role,
                  const bool export_requested)
      : RESTfulGenericInput<STORAGE>(input),
        fields(fields),
        field(field),
        field_name(field_name),
        get_url_key(get_url_key),
        role(role),
        export_requested(export_requested) {}
  RESTfulGETInput(RESTfulGenericInput<STORAGE>&& input,
                  immutable_fields_t fields,
                  const FIELD& field,
                  const std::string& field_name,
                  const Optional<url_key_t>& get_url_key,
                  const StorageRole role,
                  const bool export_requested)
      : RESTfulGenericInput<STORAGE>(std::move(input)),
        fields(fields),
        field(field),
        field_name(field_name),
        get_url_key(get_url_key),
        role(role),
        export_requested(export_requested) {}
};

// A dedicated "GETInput" for the GETs over row or col of a matrix container.
template <typename STORAGE, typename INCOMPLETE_KEY_TYPE, typename FIELD>
struct RESTfulGETRowColInput : RESTfulGenericInput<STORAGE> {
  static_assert(std::is_same<INCOMPLETE_KEY_TYPE, semantics::key_completeness::PartialRowKey>::value ||
                    std::is_same<INCOMPLETE_KEY_TYPE, semantics::key_completeness::PartialColKey>::value,
                "");

  using key_completeness_t = INCOMPLETE_KEY_TYPE;
  using immutable_fields_t = ImmutableFields<STORAGE>;

  immutable_fields_t fields;
  const FIELD& field;
  const std::string field_name;
  Optional<std::string> rowcol_get_url_key;

  RESTfulGETRowColInput(const RESTfulGenericInput<STORAGE>& input,
                        immutable_fields_t fields,
                        const FIELD& field,
                        const std::string& field_name,
                        const Optional<std::string>& rowcol_get_url_key)
      : RESTfulGenericInput<STORAGE>(input),
        fields(fields),
        field(field),
        field_name(field_name),
        rowcol_get_url_key(rowcol_get_url_key) {}
  RESTfulGETRowColInput(RESTfulGenericInput<STORAGE>&& input,
                        immutable_fields_t fields,
                        const FIELD& field,
                        const std::string& field_name,
                        const Optional<std::string>& rowcol_get_url_key)
      : RESTfulGenericInput<STORAGE>(std::move(input)),
        fields(fields),
        field(field),
        field_name(field_name),
        rowcol_get_url_key(rowcol_get_url_key) {}
};

template <typename STORAGE, typename FIELD, typename ENTRY>
struct RESTfulPOSTInput : RESTfulGenericInput<STORAGE> {
  using mutable_fields_t = MutableFields<STORAGE>;
  mutable_fields_t fields;
  FIELD& field;
  const std::string field_name;
  ENTRY& entry;

  RESTfulPOSTInput(const RESTfulGenericInput<STORAGE>& input,
                   mutable_fields_t fields,
                   FIELD& field,
                   const std::string& field_name,
                   ENTRY& entry)
      : RESTfulGenericInput<STORAGE>(input),
        fields(fields),
        field(field),
        field_name(field_name),
        entry(entry) {}
  RESTfulPOSTInput(RESTfulGenericInput<STORAGE>&& input,
                   mutable_fields_t fields,
                   FIELD& field,
                   const std::string& field_name,
                   ENTRY& entry)
      : RESTfulGenericInput<STORAGE>(std::move(input)),
        fields(fields),
        field(field),
        field_name(field_name),
        entry(entry) {}
};

template <typename STORAGE, typename FIELD, typename ENTRY, typename KEY>
struct RESTfulPUTInput : RESTfulGenericInput<STORAGE> {
  using mutable_fields_t = MutableFields<STORAGE>;
  mutable_fields_t fields;
  FIELD& field;
  const std::string field_name;
  const KEY& put_key;
  const ENTRY& entry;
  const KEY& entry_key;

  RESTfulPUTInput(const RESTfulGenericInput<STORAGE>& input,
                  mutable_fields_t fields,
                  FIELD& field,
                  const std::string& field_name,
                  const KEY& put_key,
                  const ENTRY& entry,
                  const KEY& entry_key)
      : RESTfulGenericInput<STORAGE>(input),
        fields(fields),
        field(field),
        field_name(field_name),
        put_key(put_key),
        entry(entry),
        entry_key(entry_key) {}
  RESTfulPUTInput(RESTfulGenericInput<STORAGE>&& input,
                  mutable_fields_t fields,
                  FIELD& field,
                  const std::string& field_name,
                  const KEY& put_key,
                  const ENTRY& entry,
                  const KEY& entry_key)
      : RESTfulGenericInput<STORAGE>(std::move(input)),
        fields(fields),
        field(field),
        field_name(field_name),
        put_key(put_key),
        entry(entry),
        entry_key(entry_key) {}
};

template <typename STORAGE, typename FIELD, typename KEY>
struct RESTfulDELETEInput : RESTfulGenericInput<STORAGE> {
  using mutable_fields_t = MutableFields<STORAGE>;
  mutable_fields_t fields;
  FIELD& field;
  const std::string field_name;
  const KEY& delete_key;

  RESTfulDELETEInput(const RESTfulGenericInput<STORAGE>& input,
                     mutable_fields_t fields,
                     FIELD& field,
                     const std::string& field_name,
                     const KEY& delete_key)
      : RESTfulGenericInput<STORAGE>(input),
        fields(fields),
        field(field),
        field_name(field_name),
        delete_key(delete_key) {}
  RESTfulDELETEInput(RESTfulGenericInput<STORAGE>&& input,
                     mutable_fields_t fields,
                     FIELD& field,
                     const std::string& field_name,
                     const KEY& delete_key)
      : RESTfulGenericInput<STORAGE>(std::move(input)),
        fields(fields),
        field(field),
        field_name(field_name),
        delete_key(delete_key) {}
};

// Generic wrapper to have per-`Row` and per-`Col` code collapsed into one snippet.
template <typename>
struct RowOrCallSelector;

template <>
struct RowOrCallSelector<semantics::key_completeness::PartialRowKey> {
  template <typename FIELD, typename ROW_OR_COL_KEY>
  static auto RowOrCol(FIELD&& field, ROW_OR_COL_KEY&& row_or_col_key)
      -> decltype(std::declval<FIELD>().Row(std::declval<ROW_OR_COL_KEY>())) {
    return field.Row(std::forward<ROW_OR_COL_KEY>(row_or_col_key));
  }
  template <typename FIELD>
  static auto RowsOrCols(FIELD&& field) -> decltype(std::declval<FIELD>().Rows()) {
    return field.Rows();
  }
};

template <>
struct RowOrCallSelector<semantics::key_completeness::PartialColKey> {
  template <typename FIELD, typename ROW_OR_COL_KEY>
  static auto RowOrCol(FIELD&& field, ROW_OR_COL_KEY&& row_or_col_key)
      -> decltype(std::declval<FIELD>().Col(std::declval<ROW_OR_COL_KEY>())) {
    return field.Col(std::forward<ROW_OR_COL_KEY>(row_or_col_key));
  }
  template <typename FIELD>
  static auto RowsOrCols(FIELD&& field) -> decltype(std::declval<FIELD>().Cols()) {
    return field.Cols();
  }
};

}  // namespace rest
}  // namespace storage
}  // namespace current

#endif  // CURRENT_STORAGE_API_TYPES_H
