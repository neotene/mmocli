# coding: utf-8
require 'eventmachine'
require 'em-websocket'
require 'openssl'
require 'net/http'
require 'uri'
require 'json'
require 'valid_email2'
require 'password_strength'
require 'sqlite3'
require 'securerandom'
require 'inifile'
require 'optparse'
require 'digest'

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

api_key_path = config['Server']['api_key_path']
database_name = config['Server']['database_name']
domain_name = config['Server']['domain_name']
mailgun_domain = config['Server']['mailgun_domain']
private_key_file = config['Server']['private_key_file']
cert_chain_file = config['Server']['cert_chain_file']

db = SQLite3::Database.new(database_name)

def validate_email(email)
  ValidEmail2::Address.new(email).valid?
end

def send_email(api_key_path, domain, mailgun_domain, to, subject, message)
  api_key = File.read(api_key_path).strip
  mailgun_url = "https://#{mailgun_domain}/v3/#{domain}/messages"
  params = { from: "No Reply <noreply@#{domain}>", to: to, subject: subject, text: message }

  uri = URI.parse(mailgun_url)
  http = Net::HTTP.new(uri.host, uri.port)
  http.use_ssl = true
  request = Net::HTTP::Post.new(uri.request_uri)
  request.basic_auth('api', api_key)
  request.set_form_data(params)

  response = http.request(request)

  response.code == '200' ? true : false
end

def store_user(db, email, password_hash, salt, confirmation_id)
  db.execute("INSERT INTO users (email, password_hash, salt, confirmation_id, confirmed) VALUES (?, ?, ?, ?, ?)",
             [email, password_hash, salt, confirmation_id, 0])
end

def build_email_message(domain_name, obj, confirmation_id)
  <<~MESSAGE
    MMOCLI - Registration

    Hello,

    You have successfully registered to MMOCLI.

    Your login is: #{obj['email']}

    Please click on the following link to confirm your registration:
    https://#{domain_name}/confirm?email=#{CGI.escape(obj['email'])}&confirmation_id=#{CGI.escape(confirmation_id)}

    Have fun!

    MMOCLI Team
  MESSAGE
end

def process_registration(db, api_key_path, domain_name, mailgun_domain, obj)
  if !validate_email(obj['email'])
    { 'type' => 'register', 'status' => 'error', 'message' => 'Invalid email' }
  elsif obj['password'].length < 8
    { 'type' => 'register', 'status' => 'error', 'message' => 'Password too short' }
  elsif !PasswordStrength.test(obj['email'], obj['password']).good?
    { 'type' => 'register', 'status' => 'error', 'message' => 'Password too weak' }
  else
    salt = SecureRandom.hex
    password_hash = Digest::SHA256.hexdigest(salt + obj['password'])

    existing_user = db.execute("SELECT confirmed FROM users WHERE email = ?", [obj['email']]).first

    if existing_user
      confirmed = existing_user.first.to_i

      if confirmed == 0
        { 'type' => 'register', 'status' => 'error', 'message' => 'User already exists but not confirmed' }
      else
        { 'type' => 'register', 'status' => 'error', 'message' => 'User already confirmed' }
      end
    else
      confirmation_id = SecureRandom.uuid
      store_user(db, obj['email'], password_hash, salt, confirmation_id)

      email_message = build_email_message(domain_name, obj, confirmation_id)
      if send_email(api_key_path, domain_name, mailgun_domain, obj['email'], 'MMOCLI - Registration', email_message)
        { 'type' => 'register', 'status' => 'success', 'message' => 'Registration successful' }
      else
        { 'type' => 'register', 'status' => 'error', 'message' => 'Failed to send e-mail' }
      end
    end
  end
end


EventMachine.run do
  ssl_options = { private_key_file: private_key_file, cert_chain_file: cert_chain_file, verify_peer: false }

  EventMachine::WebSocket.start(host: '0.0.0.0', port: 5465, secure: true, tls_options: ssl_options) do |ws|
    ws.onopen { puts 'Client connected via WebSocket' }

    ws.onmessage do |message|
      puts "Received message: #{message}"
      obj = JSON.parse(message)

      if obj['type'] == 'register' && !obj['email'].nil? && !obj['password'].nil?
        resp = process_registration(db, api_key_path, domain_name, mailgun_domain, obj)
      else
        resp = { 'type' => 'register', 'status' => 'error', 'message' => 'Invalid request' }
      end
      ws.send(resp.to_json)
    end

    ws.onclose { puts 'Client disconnected' }
  end
end
