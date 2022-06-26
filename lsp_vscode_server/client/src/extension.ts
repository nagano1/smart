import * as path from 'path';
import { workspace, ExtensionContext } from 'vscode';

import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
    TransportKind,
    Executable
} from 'vscode-languageclient/node';

import { statSync } from 'fs';
let client: LanguageClient;

export function activate(context: ExtensionContext) {
    /*
    let consoleApp = context.asAbsolutePath(
        path.join("server", "out", "ConsoleApplication2.exe")
    )
    */
    let consoleApp = require('path').resolve(__dirname, '../../../') + "/visual_studio_console_sln\\x64\\Release\\ConsoleApplication2.exe";
    //consoleApp = __dirname + "/ConsoleApplication2.exe";

    let serverOptions: ServerOptions = {
        command: consoleApp,
    }

    // Options to control the language client
    let clientOptions: LanguageClientOptions = {
        // Register the server for plain text documents
        //documentSelector: [{ scheme: 'file', language: 'plaintext'}],
        documentSelector: [{ scheme: 'file', language: 'smartlang'}],
        //documentSelector: [{ scheme: 'file', pattern:"*.{smt,pls,txt}" }],
        synchronize: {
            // Notify the server about file changes to '.clientrc files contained in the workspace
            fileEvents: workspace.createFileSystemWatcher('**/.clientrc')
        }
    };

    // Create the language client and start the client.
    client = new LanguageClient(
        'languageServerExample',
        'Language Server Example',
        serverOptions,
        clientOptions
    );

    // Start the client. This will also launch the server
    client.start();
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}



/*
const connection = connectToServer(hostname, path);
const client = new LanguageClient(
    "docfxLanguageServer",
    "Docfx Language Server",
    () => Promise.resolve<StreamInfo>({
        reader: connection,
        writer: connection,
    }),
    {});

private connectToServer(hostname: string, path: string): Duplex {
    const ws = new WebSocket(`ws://${hostname}/${path}`);
    return WebSocket.createWebSocketStream(ws);
}
*/