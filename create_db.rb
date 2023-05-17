# coding: utf-8
require 'sqlite3'
require 'inifile'

# Chemin du fichier de configuration
config_file = ARGV[0] || 'config.ini'

# Lecture du fichier de configuration
config = IniFile.load(config_file)

# Récupération des valeurs de configuration
database_name = config['Server']['database_name']

# Initialisation de la base de données
db = SQLite3::Database.new(database_name)

# Création de la table si elle n'existe pas déjà
db.execute <<-SQL
  CREATE TABLE IF NOT EXISTS users (
    email TEXT,
    password_hash TEXT,
    salt TEXT,
    confirmation_id TEXT,
    confirmed BOOLEAN
  )
SQL

puts 'Table users created if it did not exist.'
