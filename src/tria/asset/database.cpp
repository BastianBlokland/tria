#include "tria/asset/database.hpp"
#include "database_impl.hpp"

namespace tria::asset {

Database::Database(log::Logger* logger, fs::path rootPath) :
    m_impl{std::make_unique<DatabaseImpl>(logger, std::move(rootPath))} {}

Database::~Database() = default;

[[nodiscard]] auto Database::get(const AssetId& id) -> const Asset* { return m_impl->get(id); }

} // namespace tria::asset
