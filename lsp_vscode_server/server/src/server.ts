import {
	createConnection,
	TextDocuments,
	TextDocument,
	Diagnostic,
	DiagnosticSeverity,
	ProposedFeatures,
	InitializeParams,
	DidChangeConfigurationNotification,
	CompletionItem,
	CompletionItemKind,
	TextDocumentSyncKind,
	TextDocumentPositionParams,
	ClientCapabilities,
	ServerCapabilities,
    SemanticTokens,
    SemanticTokensBuilder,
    Range,
    Position,
} from 'vscode-languageserver';

import * as path from "path"

import {
	exec, ChildProcess
} from "child_process"
//var exec = require('child_process').exec, child

let child: ChildProcess;

function joinLine(str: string) {
	return str.split('\n').join(' ')
}

function doExec(str, cb) {
	child = exec(joinLine(str), (error, stdout, stderr) => {
		// console.log(error);
		// console.log(stderr || stdout);
		cb(error)
	})

	child.stdout.addListener('data', d => {
		console.log(d)
	})
	child.stderr.addListener('data', d => {
		console.log(d)
	})
}

async function doExecAsync(str) {
	return new Promise((resolve, reject) => {
		child = exec(joinLine(str), (error, stdout, stderr) => {
			resolve(error)
		})

		child.stdout.addListener('data', d => {
			console.warn("[" + d + "]")
		})
		child.stderr.addListener('data', d => {
			console.warn(d)
		})

	})
}


export const sendMessage = (obj: Object) => {
	let payload = JSON.stringify({ jsonrpc: "2.0", ...obj }, undefined, 1) + "\r\n"
	let msg = `Content-Length: ${payload.length}\r\n\r\n${payload}`;

	console.warn(msg)
	process.stdout.write(msg)
}

(async () => {
    //console.warn(__dirname);
    //console.warn(process.cwd());
	await doExecAsync(path.join(__dirname, "ConsoleApplication2.exe"));

	//await doExecAsync("node -e \"process.stdin.on('data',(d)=>{process.stdout.write(d+'Zatad99');}); \"");
	//child.stdin.end();
})();



export const listenToLSPClient = () => {
	let buffer = ""

	const parseAsPossible = async () => {
		//console.warn(buffer);

      
		sendMessage({
			id: "0", result: {
				capabilities: {
                    
                    semanticTokensProvider: {

                    },
					textDocumentSync: {
						// Indicate the server want the client to send
						// `textDocument/didOpen` and `textDocument/didClose` notifications
						// whenever a document opens or get closed.
						openClose: true,
						// Indicate the server want the client to send
						// `textDocument/didChange` notifications
						// whenever an open document is modified,
						// including the full text of the modified document.
						change: TextDocumentSyncKind.Full,
					},

				} as ServerCapabilities,
			}
		});

		/*
		const result = tryParseLSPMessage(buffer)
		if (result === undefined) return


		const { message, rest } = result
		buffer = rest

		onMessage(message)
		*/
	}

	process.stdin.on("data", async (data: Buffer) => {
		const chunk = data.toString()
		buffer += chunk
        child.stdin.write(chunk)
        console.warn("{{" + buffer + "}}")
        console.warn("\n")
		buffer = "";

        
		parseAsPossible()
	})
}
console.warn("sTART");

listenToLSPClient();






//old()












// function old() {

// 	// Create a connection for the server. The connection uses Node's IPC as a transport.
// 	// Also include all preview / proposed LSP features.
// 	let connection = createConnection(ProposedFeatures.all);

// 	// Create a simple text document manager. The text document manager
// 	// supports full document sync only
// 	let documents: TextDocuments = new TextDocuments();

// 	let hasConfigurationCapability: boolean = false;
// 	let hasWorkspaceFolderCapability: boolean = false;
// 	let hasDiagnosticRelatedInformationCapability: boolean = false;

// 	connection.onInitialize((params: InitializeParams) => {
// 		let capabilities = params.capabilities;

// 		// Does the client support the `workspace/configuration` request?
// 		// If not, we will fall back using global settings
// 		hasConfigurationCapability = !!(capabilities.workspace && !!capabilities.workspace.configuration);
// 		hasWorkspaceFolderCapability = !!(capabilities.workspace && !!capabilities.workspace.workspaceFolders);
// 		hasDiagnosticRelatedInformationCapability =
// 			!!(capabilities.textDocument &&
// 				capabilities.textDocument.publishDiagnostics &&
// 				capabilities.textDocument.publishDiagnostics.relatedInformation);

// 		return {
// 			capabilities: {
// 				textDocumentSync: documents.syncKind,
// 				// Tell the client that the server supports code completion
// 				completionProvider: {
// 					resolveProvider: true
// 				}
// 			}
// 		};
// 	});

// 	connection.onInitialized(() => {
// 		if (hasConfigurationCapability) {
// 			// Register for all configuration changes.
// 			connection.client.register(
// 				DidChangeConfigurationNotification.type,
// 				undefined
// 			);
// 		}
// 		if (hasWorkspaceFolderCapability) {
// 			connection.workspace.onDidChangeWorkspaceFolders(_event => {
// 				connection.console.log('Workspace folder change event received.');
// 			});
// 		}
// 	});

// 	// The example settings
// 	interface ExampleSettings {
// 		maxNumberOfProblems: number;
// 	}

// 	// The global settings, used when the `workspace/configuration` request is not supported by the client.
// 	// Please note that this is not the case when using this server with the client provided in this example
// 	// but could happen with other clients.
// 	const defaultSettings: ExampleSettings = { maxNumberOfProblems: 1000 };
// 	let globalSettings: ExampleSettings = defaultSettings;

// 	// Cache the settings of all open documents
// 	let documentSettings: Map<string, Thenable<ExampleSettings>> = new Map();

// 	connection.onDidChangeConfiguration(change => {
// 		if (hasConfigurationCapability) {
// 			// Reset all cached document settings
// 			documentSettings.clear();
// 		} else {
// 			globalSettings = <ExampleSettings>(
// 				(change.settings.languageServerExample || defaultSettings)
// 			);
// 		}

// 		// Revalidate all open text documents
// 		documents.all().forEach(validateTextDocument);
// 	});

// 	function getDocumentSettings(resource: string): Thenable<ExampleSettings> {
// 		if (!hasConfigurationCapability) {
// 			return Promise.resolve(globalSettings);
// 		}
// 		let result = documentSettings.get(resource);
// 		if (!result) {
// 			result = connection.workspace.getConfiguration({
// 				scopeUri: resource,
// 				section: 'languageServerExample'
// 			});
// 			documentSettings.set(resource, result);
// 		}
// 		return result;
// 	}

// 	// Only keep settings for open documents
// 	documents.onDidClose(e => {
// 		documentSettings.delete(e.document.uri);
// 	});

// 	// The content of a text document has changed. This event is emitted
// 	// when the text document first opened or when its content has changed.
// 	documents.onDidChangeContent(change => {
// 		console.info("here");
// 		validateTextDocument(change.document);
// 	});

// 	async function validateTextDocument(textDocument: TextDocument): Promise<void> {
// 		// In this simple example we get the settings for every validate run.
// 		let settings = await getDocumentSettings(textDocument.uri);

// 		// The validator creates diagnostics for all uppercase words length 2 and more
// 		let text = textDocument.getText();
// 		let pattern = /\b[A-Z]{2,}\b/g;
// 		let m: RegExpExecArray | null;

// 		let problems = 0;
// 		let diagnostics: Diagnostic[] = [];
// 		while ((m = pattern.exec(text)) && problems < settings.maxNumberOfProblems) {
// 			problems++;
// 			let diagnosic: Diagnostic = {
// 				severity: DiagnosticSeverity.Warning,
// 				range: {
// 					start: textDocument.positionAt(m.index),
// 					end: textDocument.positionAt(m.index + m[0].length)
// 				},
// 				message: `${m[0]} is all uppercase.`,
// 				source: 'ex'
// 			};
// 			if (hasDiagnosticRelatedInformationCapability) {
// 				diagnosic.relatedInformation = [
// 					{
// 						location: {
// 							uri: textDocument.uri,
// 							range: Object.assign({}, diagnosic.range)
// 						},
// 						message: 'Spelling matters'
// 					},
// 					{
// 						location: {
// 							uri: textDocument.uri,
// 							range: Object.assign({}, diagnosic.range)
// 						},
// 						message: 'Particularly for names'
// 					}
// 				];
// 			}
// 			diagnostics.push(diagnosic);
// 		}

// 		// Send the computed diagnostics to VSCode.
// 		connection.sendDiagnostics({ uri: textDocument.uri, diagnostics });
// 	}

// 	connection.onDidChangeWatchedFiles(_change => {
// 		// Monitored files have change in VSCode
// 		connection.console.log('We received an file change event');
// 	});

// 	// This handler provides the initial list of the completion items.
// 	connection.onCompletion(
// 		(_textDocumentPosition: TextDocumentPositionParams): CompletionItem[] => {
// 			// The pass parameter contains the position of the text document in
// 			// which code complete got requested. For the example we ignore this
// 			// info and always provide the same completion items.
// 			return [
// 				{
// 					label: 'TypeScript',
// 					kind: CompletionItemKind.Text,
// 					data: 1
// 				},
// 				{
// 					label: 'JavaScript',
// 					kind: CompletionItemKind.Text,
// 					data: 2
// 				}
// 			];
// 		}
// 	);

// 	// This handler resolves additional information for the item selected in
// 	// the completion list.
// 	connection.onCompletionResolve(
// 		(item: CompletionItem): CompletionItem => {
// 			if (item.data === 1) {
// 				(item.detail = 'TypeScript details'),
// 					(item.documentation = 'TypeScript documentation');
// 			} else if (item.data === 2) {
// 				(item.detail = 'JavaScript details'),
// 					(item.documentation = 'JavaScript documentation');
// 			}
// 			return item;
// 		}
// 	);

// 	/*
// 	connection.onDidOpenTextDocument((params) => {
// 		// A text document got opened in VSCode.
// 		// params.uri uniquely identifies the document. For documents store on disk this is a file URI.
// 		// params.text the initial full content of the document.
// 		connection.console.log(`${params.textDocument.uri} opened.`);
// 	});
// 	connection.onDidChangeTextDocument((params) => {
// 		// The content of a text document did change in VSCode.
// 		// params.uri uniquely identifies the document.
// 		// params.contentChanges describe the content changes to the document.
// 		connection.console.log(`${params.textDocument.uri} changed: ${JSON.stringify(params.contentChanges)}`);
// 	});
// 	connection.onDidCloseTextDocument((params) => {
// 		// A text document got closed in VSCode.
// 		// params.uri uniquely identifies the document.
// 		connection.console.log(`${params.textDocument.uri} closed.`);
// 	});
// 	*/

// 	// Make the text document manager listen on the connection
// 	// for open, change and close text document events
// 	documents.listen(connection);

// 	// Listen on the connection
// 	connection.listen();
// }