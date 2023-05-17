# coding: utf-8
require 'webrick'
require 'webrick/https'
require 'openssl'
require 'inifile'
require 'sinatra/base'
require 'sqlite3'
require 'cgi'
require 'optparse'

options = {}
OptionParser.new do |opts|
  opts.banner = "Usage: ruby main.rb [config_file]"

  opts.on_tail("-h", "--help", "Show this message") do
    puts opts
    exit
  end
end.parse!

config_file = ARGV[0] || 'config.ini'
config = IniFile.load(config_file)

# Récupération des valeurs de configuration
ssl_certificate_path = config['Server']['cert_chain_file']
ssl_key_path = config['Server']['private_key_file']
domain_name = config['Server']['domain_name']
database_name = config['Server']['database_name']

# Connection à la base de données
$db = SQLite3::Database.new(database_name)

# ...

# Configuration de l'application Sinatra
class MyApp < Sinatra::Base
  get '/' do
    "Hello, world!"
  end

  get '/confirm' do
    email = params['email']
    confirmation_id = params['confirmation_id']

    if email && confirmation_id
      email = CGI.unescape(email)
      confirmation_id = CGI.unescape(confirmation_id)

      $db.execute("UPDATE users SET confirmed = 1 WHERE email = ? AND confirmation_id = ?", [email, confirmation_id])
      "Confirmation successful"
    else
      "Invalid parameters"
    end
  end
end

# Configuration du serveur WEBrick avec SSL
server_options = {
  Port: 443,
  Host: '0.0.0.0',
  Logger: WEBrick::Log.new($stderr),
  DocumentRoot: '/',
  SSLEnable: true,
  SSLVerifyClient: OpenSSL::SSL::VERIFY_NONE,
  SSLPrivateKey: OpenSSL::PKey::EC.new(File.open(ssl_key_path).read),
  SSLCertificate: OpenSSL::X509::Certificate.new(File.open(ssl_certificate_path).read),
  SSLCertName: [['CN', domain_name]]
}

# Création du serveur WEBrick HTTPS avec l'application Sinatra
webrick_server = WEBrick::HTTPServer.new(server_options)
webrick_server.mount('/', Rack::Handler::WEBrick, MyApp)

# Arrêt du serveur avec le signal SIGINT (Ctrl+C)
trap('INT') { webrick_server.shutdown }

# Démarrage du serveur WEBrick HTTPS
webrick_server.start
