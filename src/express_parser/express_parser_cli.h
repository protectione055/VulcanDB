#include <memory>
#include <string>

#include "express_defs.h"
#include "express_parser.h"

class FileManager {
 public:
  static std::string read_file(const std::string& filename);

  static void write_file(const std::string& filename,
                         const std::string& content);
};

class PgSQLGenerator {
 public:
  explicit PgSQLGenerator(std::shared_ptr<ExpressParserContext> context)
      : context_(context) {}
  ~PgSQLGenerator() = default;

  std::string generate_pgsql_for_schema(std::shared_ptr<Schema> schema);

  std::string create_pgsql_for_enum(Type* type);
  std::string create_pgsql_for_entity(Entity* entity);

 private:
  std::string typecast_express_to_sql(Type* type);

 private:
  std::shared_ptr<ExpressParserContext> context_;
};