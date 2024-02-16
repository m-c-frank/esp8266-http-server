document.getElementById('alertButton').addEventListener('click', function() {
    alert('Hello! You clicked the button.');
});

// Global array to store messages
const messages = [];

document.getElementById('alertButton').addEventListener('click', function() {
    alert('Hello! You clicked the button.');
});

document.addEventListener('DOMContentLoaded', () => {
    const messageForm = document.getElementById('message-form');
    const messageInput = document.getElementById('message-input');
    const messagesContainer = document.getElementById('messages');

    // Function to add messages to the global array and display them
    function addMessage(role, content) {
        messages.push({role, content});
        const messageElement = document.createElement('div');
        messageElement.textContent = `${role}: ${content}`;
        messagesContainer.appendChild(messageElement);
    }

    messageForm.addEventListener('submit', async function(e) {
        e.preventDefault(); // Prevent form submission from reloading the page
        const message = messageInput.value.trim();
        if (message) {
            addMessage('user', message); // Display user's message in chat
            messageInput.disabled = true; // Disable input to prevent sending new messages until the current one is processed

            // Send the message and wait for the response
            await sendMessage(message);
            messageInput.disabled = false; // Re-enable input after the message is processed
            messageInput.value = ''; // Clear input field
            messageInput.focus(); // Focus on the input field
        }
    });

    // Function to send message to the server and handle the response
    async function sendMessage(content) {
        try {
            const response = await fetch('/api/query', {
                method: 'POST',
                headers: {
                    'Content-Type': 'text/plain',
                },
                body: content,
            });
            if (response.ok) {
                const data = await response.json();
                addMessage('server', data.message); // Assuming the server responds with JSON containing a message
            } else {
                console.error('Server error:', response.status);
            }
        } catch (error) {
            console.error('Network error:', error);
        }
    }

    // Function to convert messages to Markdown style chat document
    function messagesToMarkdown() {
        return messages.map(({ role, content }) => `**${role}**: ${content}`).join('\n');
    }
});
